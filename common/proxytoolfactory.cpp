/*
  proxytoolfactory.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2011-2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#include "proxytoolfactory.h"

#include <QLabel>

using namespace GammaRay;
using namespace std;

ProxyToolFactory::ProxyToolFactory(const QString &path, QObject *parent)
  : ProxyFactory<ToolFactory>(path, parent)
  , m_remotingSupported(false)
{
  m_name = value(QLatin1String("Name")).toString();
  m_supportedTypes = value(QLatin1String("X-GammaRay-Types")).toString().split(QLatin1Char(';'), QString::SkipEmptyParts);
  m_remotingSupported = value(QLatin1String("X-GammaRay-Remote"), true).toBool();
}

bool ProxyToolFactory::isValid() const
{
  return
    !id().isEmpty() &&
    !m_name.isEmpty() &&
    !m_pluginPath.isEmpty() &&
    !m_supportedTypes.isEmpty();
}

QString ProxyToolFactory::name() const
{
  return m_name;
}

QStringList ProxyToolFactory::supportedTypes() const
{
  return m_supportedTypes;
}

bool ProxyToolFactory::remotingSupported() const
{
  return m_remotingSupported;
}

void ProxyToolFactory::init(ProbeInterface *probe)
{
  loadPlugin();
  ToolFactory *fac = factory();
  if (!fac) {
    return;
  }
  Q_ASSERT(fac);
  fac->init(probe);
}

QWidget *ProxyToolFactory::createWidget(QWidget *parentWidget)
{
  loadPlugin();
  ToolFactory *fac = factory();
  if (!fac) {
    return new QLabel(tr("Plugin '%1' could not be loaded.").arg(m_pluginPath), parentWidget);
  }
  Q_ASSERT(fac);
  return fac->createWidget(parentWidget);
}
