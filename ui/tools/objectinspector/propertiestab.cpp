/*
  propertiestab.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2014 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Anton Kreuzkamp <anton.kreuzkamp@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "propertiestab.h"
#include "ui_propertiestab.h"
#include "propertywidget.h"
#include "editabletypesmodel.h"
#include "variantcontainermodel.h"

#include "ui/propertyeditor/propertyeditordelegate.h"
#include "ui/propertyeditor/propertyeditorfactory.h"
#include "ui/deferredresizemodesetter.h"

#include "common/objectbroker.h"
#include <common/propertymodel.h>
#include "common/tools/objectinspector/propertiesextensioninterface.h"

#include <QSortFilterProxyModel>
#include <QMenu>
#include <QDebug>

using namespace GammaRay;

PropertiesTab::PropertiesTab(PropertyWidget *parent)
 : QWidget(parent),
   m_ui(new Ui_PropertiesTab),
   m_interface(0),
   m_newPropertyValue(0)
{
  m_ui->setupUi(this);

  m_ui->newPropertyButton->setIcon(QIcon::fromTheme("list-add"));

  setObjectBaseName(parent->objectBaseName());
}

PropertiesTab::~PropertiesTab()
{
}

void PropertiesTab::setObjectBaseName(const QString &baseName)
{
  QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
  proxy->setDynamicSortFilter(true);
  QAbstractItemModel *model = ObjectBroker::model(baseName + '.' + "properties");
  proxy->setSourceModel(model);
  m_ui->propertyView->setModel(proxy);
  m_ui->propertyView->sortByColumn(0, Qt::AscendingOrder);
  new DeferredResizeModeSetter(
    m_ui->propertyView->header(), 0, QHeaderView::ResizeToContents);
  m_ui->propertySearchLine->setProxy(proxy);
  m_ui->propertyView->setItemDelegate(new PropertyEditorDelegate(this));
  connect(m_ui->propertyView, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(propertyContextMenu(QPoint)));
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
  connect(m_ui->propertyView, SIGNAL(doubleClicked(QModelIndex)),
          SLOT(onDoubleClick(QModelIndex)));
#endif

  EditableTypesModel *typesModel = new EditableTypesModel(this);
  proxy = new QSortFilterProxyModel(this);
  proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
  proxy->setSourceModel(typesModel);
  proxy->sort(0);
  m_ui->newPropertyType->setModel(proxy);
  connect(m_ui->newPropertyType, SIGNAL(currentIndexChanged(int)),
          this, SLOT(updateNewPropertyValueEditor()));
  updateNewPropertyValueEditor();
  connect(m_ui->newPropertyName, SIGNAL(textChanged(QString)),
          this, SLOT(validateNewProperty()));
  validateNewProperty();
  connect(m_ui->newPropertyButton, SIGNAL(clicked()),
          this, SLOT(addNewProperty()));

  m_interface =
    ObjectBroker::object<PropertiesExtensionInterface*>(baseName + ".propertiesExtension");
}

static PropertyEditorFactory::TypeId selectedTypeId(QComboBox *box)
{
  return static_cast<PropertyEditorFactory::TypeId>(
    box->itemData(box->currentIndex(), Qt::UserRole).toInt());
}

void PropertiesTab::updateNewPropertyValueEditor()
{
 delete m_newPropertyValue;

 const PropertyEditorFactory::TypeId type = selectedTypeId(m_ui->newPropertyType);

 m_newPropertyValue = PropertyEditorFactory::instance()->createEditor(type, this);
 m_ui->newPropertyLayout->insertWidget(5, m_newPropertyValue);
 m_ui->newPropertyValueLabel->setBuddy(m_newPropertyValue);
}

void PropertiesTab::validateNewProperty()
{
 Q_ASSERT(m_newPropertyValue);
 m_ui->newPropertyButton->setEnabled(!m_ui->newPropertyName->text().isEmpty());
}

void PropertiesTab::propertyContextMenu(const QPoint &pos)
{
  const QModelIndex index = m_ui->propertyView->indexAt(pos);
  if (!index.isValid()) {
    return;
  }

  const int actions = index.data(PropertyModel::ActionRole).toInt();
  if (actions == PropertyModel::NoAction) {
    return;
  }

  // TODO: check if this is a dynamic property
  QMenu contextMenu;
  if (actions & PropertyModel::Delete) {
    QAction *action = contextMenu.addAction(tr("Remove"));
    action->setData(PropertyModel::Delete);
  }
  if (actions & PropertyModel::Reset) {
    QAction *action = contextMenu.addAction(tr("Reset"));
    action->setData(PropertyModel::Reset);
  }
  if (actions & PropertyModel::NavigateTo) {
    QAction *action =
      contextMenu.addAction(tr("Show in %1").
        arg(index.data(PropertyModel::AppropriateToolRole).toString()));
    action->setData(PropertyModel::NavigateTo);
  }

  if (QAction *action = contextMenu.exec(m_ui->propertyView->viewport()->mapToGlobal(pos))) {
    const QString propertyName = index.sibling(index.row(), 0).data(Qt::DisplayRole).toString();
    switch (action->data().toInt()) {
      case PropertyModel::Delete:
        m_interface->setProperty(propertyName, QVariant());
        break;
      case PropertyModel::Reset:
        m_interface->resetProperty(propertyName);
        break;
      case PropertyModel::NavigateTo:
        QSortFilterProxyModel *proxy =
          qobject_cast<QSortFilterProxyModel*>(m_ui->propertyView->model());
        QModelIndex sourceIndex = index;
        while (proxy) {
          sourceIndex = proxy->mapToSource(sourceIndex);
          proxy = qobject_cast<QSortFilterProxyModel*>(proxy->sourceModel());
        }
        m_interface->navigateToValue(sourceIndex.row());
        break;
    }
  }
}

void PropertiesTab::onDoubleClick(const QModelIndex &index)
{
  if (index.column() != 0) {
    return;
  }

#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
  QVariant var = index.sibling(index.row(), 1).data(Qt::EditRole);

  if (!var.canConvert<QVariantList>() && !var.canConvert<QVariantHash>()) {
    return;
  }

  QTreeView *v = new QTreeView;

  VariantContainerModel *m = new VariantContainerModel(v);
  m->setVariant(var);

  v->setModel(m);
  v->show();
#endif
}

void PropertiesTab::addNewProperty()
{
  const PropertyEditorFactory::TypeId type = selectedTypeId(m_ui->newPropertyType);

  const QByteArray editorPropertyName = PropertyEditorFactory::instance()->valuePropertyName(type);
  const QVariant value = m_newPropertyValue->property(editorPropertyName);
  m_interface->setProperty(m_ui->newPropertyName->text(), value);

  m_ui->newPropertyName->clear();
  updateNewPropertyValueEditor();
}
