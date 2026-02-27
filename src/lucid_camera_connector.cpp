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

#include "lucid_camera_connector.h"
#include <ArenaApi.h>
#include <QtQml/qqml.h>
#include <QtConcurrent/QtConcurrent>

namespace qlucidcamera {

LucidCameraConnector& LucidCameraConnector::Initialize() {
  static LucidCameraConnector* SingletonObject = new LucidCameraConnector;
  return *SingletonObject;
}

LucidCameraConnector::~LucidCameraConnector() {
  std::lock_guard<std::mutex> lock(devices_mutex_);
  for (auto c : devices_)
  system_->DestroyDevice(c);
  devices_.clear();
  Arena::CloseSystem(system_);
  system_ = nullptr;
}

LucidCameraConnector::LucidCameraConnector() {
  qmlRegisterType<LucidCamera>("LucidCamera", 1, 0, "LucidCamera");
  try {
  system_ = Arena::OpenSystem();
  }
  catch (GenICam::GenericException& ge) {
  qCritical() << "\nGenICam exception thrown: " << ge.what();
  }
  catch (std::exception& ex) {
  qCritical() << "\nStandard exception thrown: " << ex.what();
  }
  catch (...) {
  qCritical() << "\nUnexpected exception thrown\n";
  }
}

QVariantList LucidCameraConnector::GetAvailableCameraInfo() {
  QVariantList ret;
  busy_ = true;
  emit busyChanged(busy_);
  try {
  system_->UpdateDevices(100);
  std::vector<Arena::DeviceInfo> devices = system_->GetDevices();
  
  for (auto info : devices) {
    bool not_connected = true;
    GenICam_3_3_LUCID::gcstring cam_name = info.UserDefinedName();
    GenICam_3_3_LUCID::gcstring model_name = info.ModelName();
    GenICam_3_3_LUCID::gcstring serial_number = info.SerialNumber();

    /*
    // 현재 연결된 카메라 체크
    for (auto cam : cameras_) {
    if (serial_number == cam->GetSerialNumber()) {
      not_connected = false;
      break;
    }
    }
    */

    if (not_connected) {
    LucidCameraInfo camera_info;
    camera_info.alias_ = QString::fromStdString(cam_name.c_str());
    camera_info.model_name_ = QString::fromStdString(model_name.c_str());
    camera_info.serial_number_ = QString::fromStdString(serial_number.c_str());
    ret.append(QVariant::fromValue(camera_info));
    }
  }

  // Sort by serial_number
  std::sort(ret.begin(), ret.end(),
    [](const QVariant& a, const QVariant& b) {
    LucidCameraInfo info_a = a.value<LucidCameraInfo>();
    LucidCameraInfo info_b = b.value<LucidCameraInfo>();
    return info_a.serial_number_.toInt() < info_b.serial_number_.toInt();
    });
  }
  catch (GenICam::GenericException& ge) {
  qCritical() << "\nGenICam exception thrown: " << ge.what();
  }
  catch (std::exception& ex) {
  qCritical() << "\nStandard exception thrown: " << ex.what();
  }
  catch (...) {
  qCritical() << "\nUnexpected exception thrown\n";
  }
  busy_ = false;
  emit busyChanged(busy_);
  return ret;
}

bool
LucidCameraConnector::ConnectDevice(LucidCameraInfo device_info) {
  bool ret = false;
  try {
  system_->UpdateDevices(100);
  std::vector<Arena::DeviceInfo> devices = system_->GetDevices();
  for (int i = 0; i < devices.size(); i++) {
    if (devices[i].SerialNumber() == device_info.serialNumber()) {
    __ConnectDevice(devices[i]);
    ret = true;
    return ret;
    }
  }
  qCritical() << "Failed to Connect Camera: Device with serial number"
        << device_info.serialNumber()
        << "not found.";
  }
  catch (GenICam::GenericException& ge) {
  qCritical() << "\nGenICam exception thrown: " << ge.what();
  }
  catch (std::exception& ex) {
  qCritical() << "\nStandard exception thrown: " << ex.what();
  }
  catch (...) {
  qCritical() << "\nUnexpected exception thrown\n";
  }
  return ret;
}

bool
LucidCameraConnector::ConnectDeviceBySerialNumber(const QString& serial_number) {
  bool ret = false;
  try {
  system_->UpdateDevices(100);
  std::vector<Arena::DeviceInfo> devices = system_->GetDevices();
  for (int i = 0; i < devices.size(); i++) {
    if (devices[i].SerialNumber() == serial_number) {
    __ConnectDevice(devices[i]);
    ret = true;
    return ret;
    }
  }
  qCritical() << "Failed to Connect Camera: Device with serial number"
        << serial_number
        << "not found.";
  }
  catch (GenICam::GenericException& ge) {
  qCritical() << "\nGenICam exception thrown: " << ge.what();
  }
  catch (std::exception& ex) {
  qCritical() << "\nStandard exception thrown: " << ex.what();
  }
  catch (...) {
  qCritical() << "\nUnexpected exception thrown\n";
  }
  return ret;
}

bool
LucidCameraConnector::ConnectDeviceByUserDefinedName(const QString& user_defined_name) {
  qDebug() << "ConnectDeviceByUserDefinedName: " << user_defined_name;
  bool ret = false;
  try {
  system_->UpdateDevices(100);
  std::vector<Arena::DeviceInfo> devices = system_->GetDevices();
  for (int i = 0; i < devices.size(); i++) {
    if (devices[i].UserDefinedName() == user_defined_name) {
    __ConnectDevice(devices[i]);
    ret = true;
    return ret;
    }
  }
  qInfo() << "Failed to Connect Camera: Device with user defined name"
        << user_defined_name
        << "not found.";
  }
  catch (GenICam::GenericException& ge) {
  qCritical() << "\nGenICam exception thrown: " << ge.what();
  }
  catch (std::exception& ex) {
  qCritical() << "\nStandard exception thrown: " << ex.what();
  }
  catch (...) {
  qCritical() << "\nUnexpected exception thrown\n";
  }
  return ret;
}

bool
LucidCameraConnector::ConnectDeviceByUserDefinedName(const QStringList& user_defined_names) {
  qDebug() << "ConnectDeviceByUserDefinedName: " << user_defined_names;
  bool ret = false;
  try {
  system_->UpdateDevices(100);
  std::vector<Arena::DeviceInfo> devices = system_->GetDevices();
  std::vector<Arena::DeviceInfo> target_devices;
  for (int i = 0; i < devices.size(); i++) {
    for (int j = 0; j < user_defined_names.size(); j++)
    if (devices[i].UserDefinedName() == user_defined_names[j]) {
    target_devices.push_back(devices[i]);
    }
  }
  if (target_devices.empty()) {
    ret = false;
  }
  else {
    __ConnectDevices(target_devices);
    ret = true;
  }
  return ret;
  }
  catch (GenICam::GenericException& ge) {
  qCritical() << "\nGenICam exception thrown: " << ge.what();
  }
  catch (std::exception& ex) {
  qCritical() << "\nStandard exception thrown: " << ex.what();
  }
  catch (...) {
  qCritical() << "\nUnexpected exception thrown\n";
  }
  return ret;
}

bool LucidCameraConnector::DisconnectDevice(LucidCamera* device) {
  qDebug() << "DisconnectDevice called";
  if (device == nullptr) {
  qWarning() << "DisconnectDevice called with nullptr";
  return false;
  }
  bool ret = false;
  busy_ = true;
  emit busyChanged(busy_);
  int disconnected_idx = -1;
  LucidCameraInfo removed_device_info;
  std::lock_guard<std::mutex> lock(devices_mutex_);
  for (int i = 0; i < devices_.size(); i++) {
  Arena::IDevice* c = devices_[i];
  if (c != nullptr && c == device->GetDevice()) {
    removed_device_info.alias_ = device->GetUserName();
    removed_device_info.model_name_ = device->GetModelName();
    removed_device_info.serial_number_ = device->GetSerialNumber();
    if (device->isRunning()) {
    device->SetStreaming(false);
    device->wait();
    }
    try {
    system_->DestroyDevice(c);
    }
    catch (const GenICam::GenericException& ge) {
    qCritical() << "\nGenICam exception thrown: " << ge.what();
    }
    catch (std::exception& ex) {
    qCritical() << "\nStandard exception thrown: " << ex.what();
    return false;
    }
    catch (...) {
    qCritical() << "\nUnexpected exception thrown\n";
    return false;
    }
    device->SetIsValid(false);
    disconnected_idx = i;
  }
  }
  if (disconnected_idx != -1) {
  devices_.erase(devices_.begin() + disconnected_idx);
  emit deviceDisconnected(removed_device_info);
  ret = true;
  }
  busy_ = false;
  emit busyChanged(busy_);
  return ret;
}

bool LucidCameraConnector::GetBusy() const {
  return busy_;
}

void LucidCameraConnector::__ConnectDevice(Arena::DeviceInfo device_info) {
  busy_ = true;
  emit busyChanged(busy_);
  QtConcurrent::run([=]() mutable {

  bool skip = false;
  for (int i = 0; i < devices_.size(); i++) {
    QString connected = QString::fromStdString(
    Arena::GetNodeValue<GenICam::gcstring>(devices_[i]->GetNodeMap(), "DeviceSerialNumber").c_str());
    QString target = QString::fromStdString(device_info.SerialNumber().c_str());
    if (connected == target) {
    skip = true;
    break;
    }
  }
  if (!skip) {
    Arena::IDevice* device = system_->CreateDevice(device_info);

    LucidCameraInfo info;
    info.alias_ = Arena::GetNodeValue<GenICam::gcstring>(device->GetNodeMap(), "DeviceUserID");
    info.model_name_ = Arena::GetNodeValue<GenICam::gcstring>(device->GetNodeMap(), "DeviceModelName");
    info.serial_number_ = Arena::GetNodeValue<GenICam::gcstring>(device->GetNodeMap(), "DeviceSerialNumber");

    devices_.push_back(device);
    emit deviceConnected(info, device);
  }

  busy_ = false;
  emit busyChanged(busy_);
  });
}

void LucidCameraConnector::__ConnectDevices(std::vector<Arena::DeviceInfo>& device_infos) {
  busy_ = true;
  emit busyChanged(busy_);
  QtConcurrent::run([=]() {
  for (auto device_info : device_infos) {

    bool skip = false;
    for (int i = 0; i < devices_.size(); i++) {
    QString connected = QString::fromStdString(
      Arena::GetNodeValue<GenICam::gcstring>(devices_[i]->GetNodeMap(), "DeviceSerialNumber").c_str());
    QString target = QString::fromStdString(device_info.SerialNumber().c_str());
    if (connected == target) {
      skip = true;
      break;
    }
    }
    if (skip) continue;

    Arena::IDevice* device = system_->CreateDevice(device_info);

    LucidCameraInfo info;
    info.alias_ = Arena::GetNodeValue<GenICam::gcstring>(device->GetNodeMap(), "DeviceUserID");
    info.model_name_ = Arena::GetNodeValue<GenICam::gcstring>(device->GetNodeMap(), "DeviceModelName");
    info.serial_number_ = Arena::GetNodeValue<GenICam::gcstring>(device->GetNodeMap(), "DeviceSerialNumber");

    devices_.push_back(device);
    emit deviceConnected(info, device);
  }
  busy_ = false;
  emit busyChanged(busy_);
  });
}

} // namespace qlucidcamera