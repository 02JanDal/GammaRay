/*
  messagemodel.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2010-2011 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Milian Wolff <milian.wolff@kdab.com>

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

#include "messagemodel.h"

#include <QtCore/QDebug>

using namespace GammaRay;

MessageModel::MessageModel(QObject *parent)
  : QAbstractTableModel(parent)
{
  qRegisterMetaType<Message>("MessageModel::Message");
}

MessageModel::~MessageModel()
{

}

void MessageModel::addMessage(const MessageModel::Message &message)
{
  ///WARNING: do not trigger *any* kind of debug output here
  ///         this would trigger an infinite loop and hence crash!

  beginInsertRows(QModelIndex(), m_messages.count(), m_messages.count());
  m_messages << message;
  endInsertRows();
}

int MessageModel::columnCount(const QModelIndex &parent) const
{
  return COLUMN_COUNT;
}

int MessageModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid()) {
    return 0;
  }

  return m_messages.count();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || index.row() > rowCount() || index.column() > columnCount()) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    const Message &msg = m_messages.at(index.row());
    if (index.column() == TypeColumn) {
      switch(msg.first) {
        case QtDebugMsg:
          return tr("Debug");
        case QtWarningMsg:
          return tr("Warning");
        case QtCriticalMsg:
          return tr("Critical");
        case QtFatalMsg:
          return tr("Fatal");
      }
    } else if (index.column() == MessageColumn) {
      ///TODO: elide
      return msg.second;
    }
  }

  return QVariant();
}

QVariant MessageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    if (section == TypeColumn) {
      return tr("Type");
    } else if (section == MessageColumn) {
      return tr("Message");
    }
  }

  return QVariant();
}

#include "messagemodel.moc"