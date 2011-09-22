/*
  mainwindow.cpp

  This file is part of Endoscope, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2010-2011 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Volker Krause <volker.krause@kdab.com>

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

#include "promolabel.h"

#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QMouseEvent>

using namespace Endoscope;

PromoLabel::PromoLabel(QWidget *parent, Qt::WindowFlags f)
: QLabel(parent, f)
{
  // load image and adapt it to user's foreground color
  QColor foreground = palette().foreground().color();
  QImage img(QString(":endoscope/kdabproducts.png"));
  img = img.alphaChannel();
  for(int i = 0; i < img.colorCount(); ++i) {
    foreground.setAlpha(qGray(img.color(i)));
    img.setColor(i, foreground.rgba());
  }
  setPixmap(QPixmap::fromImage(img));
  // done image

  setCursor(QCursor(Qt::PointingHandCursor));
  setToolTip(tr("Visit KDAB Website"));
}

void PromoLabel::mouseReleaseEvent(QMouseEvent *ev)
{
  if (ev->button() == Qt::LeftButton && ev->modifiers() == Qt::NoModifier) {
    QDesktopServices::openUrl(QUrl("http://www.kdab.com"));
    ev->accept();
    return;
  }

  QLabel::mouseReleaseEvent(ev);
}

#include "promolabel.moc"
