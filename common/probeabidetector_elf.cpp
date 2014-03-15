/*
  probeabidetector_elf.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2014 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#include "config-gammaray.h"

#include "probeabidetector.h"
#include "probeabi.h"

#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QString>
#include <QStringList>

using namespace GammaRay;

ProbeABI ProbeABIDetector::abiForExecutable(const QString& path) const
{
  return ProbeABI::fromString(GAMMARAY_PROBE_ABI);
}


static QString qtCoreFromProc(qint64 pid)
{
  const QString mapsPath = QString("/proc/%1/maps").arg(pid);
  QFile f(mapsPath);
  if (!f.open(QFile::ReadOnly))
    return QString();

  forever {
    const QByteArray line = f.readLine();
    if (line.isEmpty())
      break;
    if (line.contains("QtCore") || line.contains("Qt5Core")) {
      const int pos = line.indexOf('/');
      if (pos <= 0)
        continue;
      return QString::fromLocal8Bit(line.mid(pos).trimmed());
    }
  }

  return QString();
}

ProbeABI ProbeABIDetector::abiForProcess(qint64 pid) const
{
  const QString qtCorePath = qtCoreFromProc(pid);
  if (!qtCorePath.isEmpty())
    return abiForQtCore(qtCorePath);

  return ProbeABI(); // TODO non-/proc fallback
}


static ProbeABI qtVersionFromFileName(const QString &path)
{
  ProbeABI abi;

  const QStringList parts = path.split('.');
  if (parts.size() < 4 || parts.at(parts.size() - 4) != "so")
    return abi;

  abi.setQtVersion(parts.at(parts.size() - 3).toInt(), parts.at(parts.size() - 2).toInt());
  return abi;
}

static ProbeABI qtVersionFromExec(const QString &path)
{
  ProbeABI abi;

  // yep, you can actually execute QtCore.so...
  QProcess proc;
  proc.setReadChannelMode(QProcess::SeparateChannels);
  proc.setReadChannel(QProcess::StandardOutput);
  proc.start(path);
  proc.waitForFinished();
  const QByteArray line = proc.readLine();
  const int pos = line.lastIndexOf(' ');
  const QList<QByteArray> version = line.mid(pos).split('.');
  if (version.size() < 3)
    return abi;

  abi.setQtVersion(version.at(0).toInt(), version.at(1).toInt());

  return abi;
}

ProbeABI ProbeABIDetector::detectAbiForQtCore(const QString& path) const
{
  // try to find the version
  ProbeABI abi = qtVersionFromFileName(path);
  if (!abi.hasQtVersion())
    abi = qtVersionFromExec(path);

  // TODO: detect architecture
  abi.setArchitecture("TODO");

  return abi;
}