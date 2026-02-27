/*
 * libqlucidcamera-cpp
 * Copyright (C) 2026 HIL Lab. Inc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Contact: software@hillab.co.kr
 */

#ifndef LIBQLUCIDCAMERA_LUCID_CAMERA_CONNECTOR_H
#define LIBQLUCIDCAMERA_LUCID_CAMERA_CONNECTOR_H

#include <string>
#include <iomanip>
#include <vector>
#include <mutex>
#include <atomic>
#include <QObject>
#include <QVariant>
#include <QVariantList>

#include "lucid_camera.h"

#define LUCID_CAMERA std::shared_ptr<LucidCamera>
#define CREATE_LUCID_CAMERA std::make_shared<LucidCamera>()

namespace Arena {
class IDevice;
class ISystem;
class DeviceInfo;
}

namespace qlucidcamera {

class LucidCameraInfo {
  Q_GADGET
  Q_PROPERTY(QString alias READ alias CONSTANT)
  Q_PROPERTY(QString modelName READ modelName CONSTANT)
  Q_PROPERTY(QString serialNumber READ serialNumber CONSTANT)

public:
  QString alias() const { return alias_; }
  QString modelName() const { return model_name_; }
  QString serialNumber() const { return serial_number_; }

  QString alias_;
  QString model_name_;
  QString serial_number_;
};

class LucidCameraConnector : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool busy READ GetBusy NOTIFY busyChanged)

Q_SIGNALS:
  void deviceConnected(LucidCameraInfo device_info, Arena::IDevice* device);
  void deviceDisconnected(LucidCameraInfo device_info);
  void busyChanged(bool busy);

public:
  static LucidCameraConnector& Initialize();
  ~LucidCameraConnector();

private:
  LucidCameraConnector();

public: // Methods for camera controls
  Q_INVOKABLE QVariantList GetAvailableCameraInfo();
  Q_INVOKABLE bool ConnectDevice(LucidCameraInfo device_info);
  Q_INVOKABLE bool ConnectDeviceBySerialNumber(const QString& serial_number);
  Q_INVOKABLE bool ConnectDeviceByUserDefinedName(const QString& user_defined_name);
  Q_INVOKABLE bool ConnectDeviceByUserDefinedName(const QStringList& user_defined_names);
  Q_INVOKABLE bool DisconnectDevice(LucidCamera* device);
  bool GetBusy() const;

private:
  void __ConnectDevice(Arena::DeviceInfo device_info);
  void __ConnectDevices(std::vector<Arena::DeviceInfo>& device_infos);

private:
  Arena::ISystem* system_;
  std::vector< Arena::IDevice* > devices_;
  std::mutex devices_mutex_;
  std::atomic_bool busy_{ false };
};

} // namespace qlucidcamera

Q_DECLARE_METATYPE(qlucidcamera::LucidCameraInfo)

#endif // LIBQLUCIDCAMERA_LUCID_CAMERA_CONNECTOR_H