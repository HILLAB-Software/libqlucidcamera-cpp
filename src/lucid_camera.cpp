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

#define LUCID_CAM_BUFFER_SIZE 20

#include "lucid_camera.h"
#include <ArenaApi.h>
#include <GenApi/Filestream.h>
#include <SaveApi.h>
#include <QDebug>

namespace qlucidcamera {

template <typename T>
T LucidCamera::__GetNodeValue(const std::string& name) {
  try {
    if (!GetIsValid()) return 0;
    if (device_ == nullptr)
      throw std::runtime_error(
        "GetNodeValue " + name + " failed: Camera is not specified.");
    if (!device_->IsConnected()) {
      qCritical() << "Camera GetNodeValue '" << name.c_str() << "' failed: Device connection lost";
      emit deviceLostExceptionThrown("");
      return 0;
    }
    return Arena::GetNodeValue<T>(device_->GetNodeMap(), name.c_str());
  }
  catch (const std::exception& e) {
    qCritical() << e.what();
    return 0;
  }
  catch (GenICam::GenericException& e) {
    qCritical() << e.what();
    return 0;
  }
}

template <typename T>
T LucidCamera::__GetNodeMax(const std::string& name) {
  try {
    if (!GetIsValid()) return 0;
    if (device_ == nullptr)
      throw std::runtime_error(
        "GetNodeMax " + name + " failed: Camera is not specified.");
    if (!device_->IsConnected()) {
      qCritical() << "Camera GetNodeMax '" << name.c_str() << "' failed: Device connection lost";
      emit deviceLostExceptionThrown("");
      return 0;
    }
    return Arena::GetNodeMax<T>(device_->GetNodeMap(), name.c_str());
  }
  catch (const std::exception& e) {
    qCritical() << e.what();
    return 0;
  }
  catch (GenICam::GenericException& e) {
    qCritical() << e.what();
    return 0;
  }
}

template <typename T>
T LucidCamera::__GetNodeMin(const std::string& name) {
  try {
    if (!GetIsValid()) return 0;
    if (device_ == nullptr)
      throw std::runtime_error(
        "GetNodeMin " + name + " failed: Camera is not specified.");
    if (!device_->IsConnected()) {
      qCritical() << "Camera GetNodeMin '" << name.c_str() << "' failed: Device connection lost";
      emit deviceLostExceptionThrown("");
      return 0;
    }
    return Arena::GetNodeMin<T>(device_->GetNodeMap(), name.c_str());
  }
  catch (const std::exception& e) {
    qCritical() << e.what();
    return 0;
  }
  catch (GenICam::GenericException& e) {
    qCritical() << e.what();
    return 0;
  }
}

template <typename T>
void LucidCamera::__SetNodeValue(const std::string& name, const T& value) {
  try {
    if (!GetIsValid()) return;
    if (device_ == nullptr)
      throw std::runtime_error(
        "SetNodeValue " + name + " failed: Camera is not specified.");
    if (!device_->IsConnected()) {
      qCritical() << "Camera SetNodeValue '" << name.c_str() << "' failed: Device connection lost";
      emit deviceLostExceptionThrown("");
      return;
    }
    return Arena::SetNodeValue<T>(device_->GetNodeMap(), name.c_str(), value);
  }
  catch (const std::exception& e) {
    qCritical() << e.what();
  }
  catch (GenICam::GenericException& e) {
    qCritical() << e.what();
  }
}

void LucidCamera::__ExecuteNode(const std::string& name) {
  try {
    if (!GetIsValid()) return;
    if (device_ == nullptr)
      throw std::runtime_error(
        "ExecuteNode " + name + " failed: Camera is not specified.");
    if (!device_->IsConnected()) {
      qCritical() << "Camera ExecuteNode '" << name.c_str() << "' failed: Device connection lost";
      emit deviceLostExceptionThrown("");
      return;
    }
    Arena::ExecuteNode(device_->GetNodeMap(), name.c_str());
  }
  catch (const std::exception& e) {
    qCritical() << e.what();
  }
  catch (GenICam::GenericException& e) {
    qCritical() << e.what();
  }
}

/// 이 객체가 지워지기 전, device_ 를 받아온 뒤 상위에서 Arena::ISystem::DestroyDevice 호출로 제거해야 함.
LucidCamera::LucidCamera(QObject* parent)
: QThread(parent) {
}

LucidCamera::~LucidCamera() {
  if (is_streaming_) {
    SetStreaming(false);
    QThread::wait();
  }
}

void LucidCamera::SetDevice(Arena::IDevice* created_device) {
  try {
    device_ = created_device;

    __SetNodeValue<bool>("PtpEnable", false);

    // Set acquisition mode
    //    Set acquisition mode before starting the stream. Starting the stream
    //    requires the acquisition mode to be set beforehand. The acquisition
    //    mode controls the number of images a device acquires once the stream
    //    has been started. Setting the acquisition mode to 'Continuous' keeps
    //    the stream from stopping.
    __SetNodeValue<GenICam::gcstring>(
      "AcquisitionMode",
      "Continuous");

    // enable stream auto negotiate packet size
    Arena::SetNodeValue<bool>(
      device_->GetTLStreamNodeMap(),
      "StreamAutoNegotiatePacketSize",
      true);

    // enable stream packet resend
    Arena::SetNodeValue<bool>(
      device_->GetTLStreamNodeMap(),
      "StreamPacketResendEnable",
      true);

    // Stream settings
    GenApi::INodeMap* stream_map = device_->GetTLStreamNodeMap();

    // static_cast<GenApi::CIntegerPtr>(stream_map->GetNode("StreamAnnouncedBufferCount"))->SetValue(10);

    GenApi::CEnumerationPtr stream_mode = stream_map->GetNode("StreamBufferHandlingMode");
    stream_mode->SetIntValue(stream_mode->GetEntryByName("NewestOnly")->GetValue());

    is_streaming_ = false;

    __SetNodeValue<GenICam::gcstring>(
      "ExposureAuto",
      "Off");

    __SetNodeValue<GenICam::gcstring>(
      "GainAuto",
      "Off");

    /*
    if (!__GetNodeValue<bool>("PtpEnable")) {
      __SetNodeValue<bool>(
        
        "AcquisitionFrameRateEnable",
        true);
    }
    */
    
    __SetNodeValue<GenICam::gcstring>(
      "PixelFormat",
      "Mono8");

    //gen_exposure_ = device_->GetNodeMap()->GetNode("ExposureTime");
    //gen_gain_ = device_->GetNodeMap()->GetNode("Gain");
    //gen_fps_ = device_->GetNodeMap()->GetNode("AcquisitionFrameRate");
    //gen_ptp_fps_ = device_->GetNodeMap()->GetNode("PTPSyncFrameRate");
    //gen_roi_width_ = device_->GetNodeMap()->GetNode("Width");
    //gen_roi_height_ = device_->GetNodeMap()->GetNode("Height");
    //gen_offset_x_ = device_->GetNodeMap()->GetNode("OffsetX");
    //gen_offset_y_ = device_->GetNodeMap()->GetNode("OffsetY");


    // qDebug() << "LUCID CAMERA SET DEVICE DONE";

    SetIsValid(true);
  }
  catch (GenICam::GenericException& ge) {
    qCritical() << ge.what();
  }
  catch (std::exception& ex) {
    qCritical() << ex.what();
  }
}

void LucidCamera::ZoomRoi(const int zoom_factor) {

    if (zoom_factor < 1)
      return;
    bool Stream = false;
    if (is_streaming_) {
      Stream = true;
      SetStreaming(false);
      QThread::wait();
    }
    SetOffsetX(0);
    SetOffsetY(0);

    const double ResizeFactor = 1.0 / (double)zoom_factor;
    int w = GetMaxWidth() * ResizeFactor;
    int h = GetMaxHeight();
    const int minW = GetMinWidth();
    const int minH = GetMinHeight();

    if ((w - minW) % minW != 0)
      w += (minW - (w - minW) % minW);
    if ((h - minH) % minH != 0)
      h += (minH - (h - minH) % minH);

    int offsetX = (GetMaxWidth() - w) / 2;
    int offsetY = (GetMaxHeight() - h) / 2;

    SetWidth(w);
    SetHeight(h);

    if (offsetX % minW != 0)
      offsetX -= (offsetX % minW);
    if (offsetY % minH != 0)
      offsetY -= (offsetY % minH);

    SetOffsetX(offsetX);
    SetOffsetY(offsetY);
    if (Stream)
      SetStreaming(true);
}

void LucidCamera::SetExposure(const double value) {
    const double min = GetMinExposure();
    const double max = GetMaxExposure();
    double input_value = value;
    if (input_value < min) input_value = min;
    else if (input_value > max) input_value = max;
    __SetNodeValue<GenICam::gcstring>(
      "ExposureAuto",
      "Off");
    __SetNodeValue<double>(
      "ExposureTime", input_value);
    emit exposureChanged(input_value);
}

void LucidCamera::SetGain(const double value) {
    const double min = GetMinGain();
    const double max = GetMaxGain();
    double input_value = value;
    if (input_value < min) input_value = min;
    else if (input_value > max) input_value = max;
    __SetNodeValue<GenICam::gcstring>(
      "GainAuto",
      "Off");
    __SetNodeValue<double>(
      "Gain", input_value);
    emit gainChanged(input_value);
}

void LucidCamera::SetFrameRate(const double value) {
  double input_value = value;
  const bool ptp = GetAcquisitionStartMode() == ACQUISITIONSTARTMODE_PTPSYNC;
  if (ptp && GetTriggerMode()) {
    qWarning() << "SetFrameRate warning: When trigger mode, ptp fps is not writable.";
    return;
  }
  const double min = __GetNodeMin<double>("AcquisitionFrameRate");
  const double max = __GetNodeMax<double>("AcquisitionFrameRate");

  if (input_value < min + 0.1) input_value = min;
  else if (input_value > max - 0.1) input_value = max;

  if (!__GetNodeValue<bool>("AcquisitionFrameRateEnable")) {
    __SetNodeValue<bool>("AcquisitionFrameRateEnable", true);
  }
  if (abs(GetFrameRate() - value) >= 0.1) {
    __SetNodeValue<double>("AcquisitionFrameRate", input_value);
  }
  if (ptp) {
    const double ptp_value = input_value - 0.02;
    if (abs(__GetNodeValue<double>("PTPSyncFrameRate") - ptp_value) >= 0.1) {
      __SetNodeValue<double>("PTPSyncFrameRate", input_value - 0.02);
    }
  }
  emit fpsChanged(input_value);
  emit exposureRangeChanged(GetMinExposure(), GetMaxExposure());
  /*
  const double max_exposure_limit = GetMaxExposureAvailable();
  if (user_defined_exposure_max_ > max_exposure_limit)
    SetMaxExposure(max_exposure_limit);
  emit exposureRangeChanged(GetMinExposure(), GetMaxExposure());
  */
  /*
  //__SetNodeValue<double>("PTPSyncFrameRate", value);
  
  double input_value = value;
  const bool ptp = GetAcquisitionStartMode() == ACQUISITIONSTARTMODE_PTPSYNC;
  if (ptp && GetTriggerMode()) {
    qWarning() << "SetFrameRate warning: When trigger mode, ptp fps is not writable.";
    return;
  }
  const double min = __GetNodeMin<double>(
    ptp ? "PTPSyncFrameRate" : "AcquisitionFrameRate");
  const double max = ptp ?
    __GetNodeValue<double>("AcquisitionFrameRate") : 
    __GetNodeMax<double>("AcquisitionFrameRate");
  // qDebug() << "SET FRAME RATE, Available max =" << max;
  if (input_value < min + 0.1) input_value = min;
  else if (input_value > max - 0.1) input_value = max;

  if (abs(GetFrameRate() - value) < 0.1)
    return;

  if (ptp) {
    const bool streaming = GetIsStreaming();
    if (streaming) SetStreaming(false);
    // qDebug() << "BEFORE SET FRAME RATE, APPLYING VALUE=" << input_value;
    __SetNodeValue<double>(
      "PTPSyncFrameRate", input_value);
    // qDebug() << "SET FRAME DONE";
    if (streaming) SetStreaming(true);
  }
  else {
    if (!__GetNodeValue<bool>(
      "AcquisitionFrameRateEnable"))
      __SetNodeValue<bool>(
        "AcquisitionFrameRateEnable", true);
    __SetNodeValue<double>(
      "AcquisitionFrameRate", input_value);
  }
  emit fpsChanged(input_value);
  const double max_exposure_limit = GetMaxExposureAvailable();
  if (user_defined_exposure_max_ > max_exposure_limit)
    SetMaxExposure(max_exposure_limit);
  emit exposureRangeChanged(GetMinExposure(), GetMaxExposure());
  
  //fpsChanged(value); 
  */
}

Arena::IDevice* LucidCamera::GetDevice() {
  return device_;
}

QString LucidCamera::GetModelName() {
  return __GetNodeValue<GenICam::gcstring>(
    "DeviceModelName").c_str();
}

QString LucidCamera::GetSerialNumber() {
  return __GetNodeValue<GenICam::gcstring>(
    "DeviceSerialNumber").c_str();
}

QString LucidCamera::GetMacAddress() {
  return QString::number(__GetNodeValue<int64_t>(
    "GevMACAddress"));
}

QString LucidCamera::GetUserName() {
  return __GetNodeValue<GenICam::gcstring>(
    "DeviceUserID").c_str();
}

double LucidCamera::GetMaxExposure() {
  return __GetNodeMax<double>("ExposureTime");
}

double LucidCamera::GetMinExposure() { 

    return __GetNodeMin<double>(
      "ExposureTime");
}

double LucidCamera::GetMaxGain() {
  return __GetNodeMax<double>("Gain");
}

double LucidCamera::GetMinGain() {

    return __GetNodeMin<double>(
      "Gain");
}

double LucidCamera::GetExposure() {

    return __GetNodeValue<double>(
      "ExposureTime");
}

double LucidCamera::GetGain() {

    return __GetNodeValue<double>(
      "Gain");
}

double LucidCamera::GetMinFrameRate() {

    if (GetPtpEnable() && GetTriggerMode()) {
      qWarning() << "Camera GetMinFrameRate warning: When trigger mode, ptp fps is not readable.";
      return __GetNodeMin<double>(
        "AcquisitionFrameRate");
    }
    if (GetPtpEnable()) {
      return __GetNodeMin<double>(
        "PTPSyncFrameRate");
    }
    else {
      return __GetNodeMin<double>(
        "AcquisitionFrameRate");
    }
}

double LucidCamera::GetMaxFrameRate() {
  const bool ptp = GetPtpEnable();
  if (ptp && GetTriggerMode()) {
    qWarning() << "Camera GetMaxFrameRate warning: When trigger mode, ptp fps is not readable.";
    return __GetNodeMax<double>(
      "AcquisitionFrameRate");
  }
  return __GetNodeMax<double>(
    "AcquisitionFrameRate");
  /*
  if (ptp) {
    const float max = __GetNodeMax<double>("AcquisitionFrameRate");
    //qDebug() << "GET MAX FRAME RATE, Max fps =" << max;
    __SetNodeValue<double>("AcquisitionFrameRate", max - 0.001);
    //qDebug() << "GET MAX FRAME RATE, After fps =" << __GetNodeValue<double>("AcquisitionFrameRate");
    // __SetNodeValue<double>("AcquisitionFrameRate", __GetNodeMax<double>("AcquisitionFrameRate"));
    return __GetNodeValue<double>(
      "AcquisitionFrameRate");
  }
  else {
    return __GetNodeMax<double>(
      "AcquisitionFrameRate");
  }
  */
}

double LucidCamera::GetFrameRate() {
  /*
    if (GetPtpEnable() && GetTriggerMode()) {
      qWarning() << "Camera GetFrameRate warning: When trigger mode, ptp fps is not readable.";
      return __GetNodeValue<double>(
        "AcquisitionFrameRate");
    }
    if (GetAcquisitionStartMode() == ACQUISITIONSTARTMODE_PTPSYNC) {
      return __GetNodeValue<double>(
        "PTPSyncFrameRate");
    }
    else {
      return __GetNodeValue<double>(
        "AcquisitionFrameRate");
    }
    */
  return __GetNodeValue<double>(
    "AcquisitionFrameRate");
}

int LucidCamera::GetWidth() {

    return static_cast<int>(__GetNodeValue<int64_t>(
      "Width"));
}

int LucidCamera::GetMaxWidth() {

    return static_cast<int>(__GetNodeMax<int64_t>(
      "Width"));
}

int LucidCamera::GetMinWidth() {

    return static_cast<int>(__GetNodeMin<int64_t>(
      "Width"));
}

int LucidCamera::GetHeight() {

    return static_cast<int>(__GetNodeValue<int64_t>(
      "Height"));
}

int LucidCamera::GetMaxHeight() {

    return static_cast<int>(__GetNodeMax<int64_t>(
      "Height"));
}

int LucidCamera::GetMinHeight() {

    return static_cast<int>(__GetNodeMin<int64_t>(
      "Height"));
}

int LucidCamera::GetOffsetX() {
    return static_cast<int>(__GetNodeValue<int64_t>(
      "OffsetX"));
}

int LucidCamera::GetMaxOffsetX() {
    return static_cast<int>(__GetNodeMax<int64_t>(
      "OffsetX"));
}

int LucidCamera::GetOffsetY() {
    return static_cast<int>(__GetNodeValue<int64_t>(
      "OffsetY"));
}

int LucidCamera::GetMaxOffsetY() {
    return static_cast<int>(__GetNodeMax<int64_t>(
      "OffsetY"));
}

bool LucidCamera::GetIsStreaming() {
  return is_streaming_;
}

bool LucidCamera::GetTriggerMode() {
  return is_trigger_mode_;
}

int LucidCamera::GetTriggerSource() {
  if (GetAcquisitionStartMode() == ACQUISITIONSTARTMODE_PTPSYNC) return 0;
    GenICam::gcstring str
      = __GetNodeValue<GenICam::gcstring>(
        "TriggerSource");
    if (str == "Software") return TRIGGERSOURCE_SOFTWARE;
    else if (str == "Line0") return TRIGGERSOURCE_LINE0;
    else if (str == "Line2") return TRIGGERSOURCE_LINE2;
    else if (str == "Line3") return TRIGGERSOURCE_LINE3;
    else if (str == "Action0") return TRIGGERSOURCE_ACTION0;
    else return TRIGGERSOURCE_UNDEFINED;
}

int LucidCamera::GetTriggerActivation() {
  if (GetAcquisitionStartMode() == ACQUISITIONSTARTMODE_PTPSYNC) return 0;
    GenICam::gcstring str 
      = __GetNodeValue<GenICam::gcstring>(
        "TriggerActivation");
    if (str == "RisingEdge") return TRIGGERACTIVATION_RISINGEDGE;
    else if (str == "FallingEdge") return TRIGGERACTIVATION_FALLINGEDGE;
    else if (str == "AnyEdge") return TRIGGERACTIVATION_ANYEDGE;
    else if (str == "LevelHigh") return TRIGGERACTIVATION_LEVELHIGH;
    else if (str == "LevelLow") return TRIGGERACTIVATION_LEVELLOW;
    else return TRIGGERACTIVATION_UNDEFINED;
}

bool LucidCamera::GetPtpEnable() {
    return __GetNodeValue<bool>("PtpEnable");
}

QString LucidCamera::GetPtpStatus() {
  return __GetNodeValue<GenICam::gcstring>("PtpStatus").c_str();
}

int LucidCamera::GetAcquisitionStartMode() {
  const GenICam::gcstring str 
    = __GetNodeValue<GenICam::gcstring>("AcquisitionStartMode");
  int ret = -1;
  if (str == "Normal") ret = ACQUISITIONSTARTMODE_NORMAL;
  else if (str == "LowLatency") ret = ACQUISITIONSTARTMODE_LOWLATENCY;
  else if (str == "PTPSync") ret = ACQUISITIONSTARTMODE_PTPSYNC;
  return ret;
}

int LucidCamera::GetStreamChannelPacketDelay() {
    return static_cast<int>(
      __GetNodeValue<int64_t>("GevSCPD"));
}

int LucidCamera::GetStreamChannelFrameTransmissionDelay() {
    return static_cast<int>(
      __GetNodeValue<int64_t>("GevSCFTD"));
}

int LucidCamera::GetStreamChannelPacketSize() {
    return static_cast<int>(
      __GetNodeValue<int64_t>("GevSCPSPacketSize"));
}

int LucidCamera::GetPixelFormat() {
  return pixel_format_;
}

double LucidCamera::GetPhysicalPixelSize() {
  return __GetNodeValue<double>("PhysicalPixelSize");
}

bool LucidCamera::GetVoltageExternalEnabled() {
  // Check line4 -> VoltageExternalEnable == true
  std::lock_guard<std::mutex> lock(line_selector_mutex_);
  __SetLineSelector(LINESELECTOR_LINE4);
  return __GetNodeValue<bool>("VoltageExternalEnable");
}

bool LucidCamera::GetOptoStrobeOutputEnabled() {
  // Check line1 -> LineSource == ExposureActive
  std::lock_guard<std::mutex> lock(line_selector_mutex_);
  __SetLineSelector(LINESELECTOR_LINE1);
  return __GetLineSource() == LINESOURCE_EXPOSUREACTIVE;
}

bool LucidCamera::SetStreaming(const bool flag) {
  if (!is_valid_) {
    qCritical() << "SetStreaming error: Object has disconnected.";
    return false;
  }
  if (flag) {
    if (is_streaming_)
      return false;
    if (device_->IsConnected()) {
      device_->StartStream(LUCID_CAM_BUFFER_SIZE);
      is_streaming_ = true;
      QThread::start(QThread::TimeCriticalPriority);
      return true;
    }
    else {
      qCritical() << "SetStreaming failed: Device connection lost";
      emit deviceLostExceptionThrown("");
      return false;
    }
  }
  else {
    if (!is_streaming_)
      return false;
    is_streaming_ = false;
    QThread::wait();
    return true;
  }
}


bool LucidCamera::SetTriggerMode(const bool flag) {
  // When ptp enabled, triggering acquisition requires "AcquisitionStartMode" not to be "PTPSync",
  // but only "PTPEnabled" to be "True".
  // Otherwise, the "AcquisitionStartMode" should be "PTPSync" when continuous acquisition.
  // https://support.thinklucid.com/app-note-bandwidth-sharing-in-multi-camera-systems/

  // can modified when streaming is off
  if (flag == is_trigger_mode_) return flag;
  if (is_streaming_) {
    qCritical() << "Camera::SetTriggerMode error: "
      "Cannot change trigger mode when camera is streaming.";
    return false;
  }
  if (flag && GetPtpEnable()) { 
    // AcquisitionStartMode should not be "PTPSync" before set trigger.
    __SetNodeValue<GenICam::gcstring>(
      "AcquisitionStartMode", "Normal");
  }
  if (flag) {
    SetFrameRate(GetMaxFrameRate());
  }
  __SetNodeValue<GenICam::gcstring>(
    "TriggerMode",
    flag ? "On" : "Off");
  if (!flag && GetPtpEnable()) {
    __SetNodeValue<GenICam::gcstring>(
      "AcquisitionStartMode", "PTPSync");
  }
  is_trigger_mode_ = flag;
  emit triggerModeChanged(flag);
  return true;
}

void LucidCamera::SetTriggerSource(int trigger_source_type) {
  if (GetTriggerSource() == trigger_source_type) return;
  if (GetAcquisitionStartMode() == ACQUISITIONSTARTMODE_PTPSYNC) return;
  if (GetPtpEnable()) return;
  std::string str; 
  switch (trigger_source_type) {
  case TRIGGERSOURCE_SOFTWARE: str = "Software"; break;
  case TRIGGERSOURCE_LINE0: str = "Line0"; break; 
  case TRIGGERSOURCE_LINE2: str = "Line2"; break;
  case TRIGGERSOURCE_LINE3: str = "Line3"; break;
  case TRIGGERSOURCE_ACTION0: str = "Action0"; break; 
  }
  __SetNodeValue<GenICam::gcstring>(
      
    "TriggerSource",
    str.c_str()
  );
  emit triggerSourceChanged(trigger_source_type);
}

void LucidCamera::SetTriggerActivation(int trigger_activation_type) {
  if (GetTriggerActivation() == trigger_activation_type) return;
  if (GetAcquisitionStartMode() == ACQUISITIONSTARTMODE_PTPSYNC) return;
    std::string str;
    switch (trigger_activation_type) {
    case TRIGGERACTIVATION_RISINGEDGE: str = "RisingEdge"; break;
    case TRIGGERACTIVATION_FALLINGEDGE: str = "FallingEdge"; break;
    case TRIGGERACTIVATION_ANYEDGE: str = "AnyEdge"; break;
    case TRIGGERACTIVATION_LEVELHIGH: str = "LevelHigh"; break;
    case TRIGGERACTIVATION_LEVELLOW: str = "LevelLow"; break;
    }
    __SetNodeValue<GenICam::gcstring>(
      "TriggerActivation",
      str.c_str()
      );
    if (GetPtpEnable()
      && (trigger_activation_type == TRIGGERACTIVATION_LEVELHIGH &&
        trigger_activation_type == TRIGGERACTIVATION_LEVELLOW)) {
      qWarning() << "Camera warning:"
        << "TRIGGERACTIVATION_LEVELHIGH and TRIGGERACTIVATION_LEVELLOW"
        << "won't work in PTP mode.";
    }
    emit triggerActivationChanged(trigger_activation_type);
}

void LucidCamera::SoftwareTrigger() {
  __ExecuteNode("TriggerSoftware");
}

void LucidCamera::DeviceReset() {
  __ExecuteNode("DeviceReset");
  emit deviceLostExceptionThrown("");
}

void LucidCamera::SetUserName(const QString value) {
  qDebug() << "SetUserName " << value << "called";
  __SetNodeValue<GenICam::gcstring>(
      "DeviceUserID", 
      value.toStdString().c_str()
  );
  emit userNameChanged(value);
}

void LucidCamera::SetWidth(const int value) {
    int input_value = value;
    const int min = GetMinWidth();
    const int max = GetMaxWidth();
    if (input_value < min) input_value = min;
    else if (input_value > max) input_value = max;
    __SetNodeValue<int64_t>(
      "Width", static_cast<int64_t>(input_value));
    emit roiWidthChanged(input_value);
}

void LucidCamera::SetHeight(const int value) {
    int input_value = value;
    const int min = GetMinHeight();
    const int max = GetMaxHeight();
    if (input_value < min) input_value = min;
    else if (input_value > max) input_value = max;
    __SetNodeValue<int64_t>(
      "Height", static_cast<int64_t>(input_value));
    emit roiHeightChanged(input_value);
}

void LucidCamera::SetOffsetX(const int value) {
    int input_value = value;
    const int min = 0;
    const int max = GetMaxOffsetX();
    if (input_value < min) input_value = min;
    else if (input_value > max) input_value = max;
    __SetNodeValue<int64_t>(
      "OffsetX", static_cast<int64_t>(input_value));
}

void LucidCamera::SetOffsetY(const int value) {
    int input_value = value;
    const int min = 0;
    const int max = GetMaxOffsetY();
    if (input_value < min) input_value = min;
    else if (input_value > max) input_value = max;
    __SetNodeValue<int64_t>(
      "OffsetY", static_cast<int64_t>(input_value));
}

void LucidCamera::SetPtpEnable(const bool flag) {
  if (flag) {
    // Set max before ptp enable
    if (!__GetNodeValue<bool>(
      "AcquisitionFrameRateEnable"))
      __SetNodeValue<bool>(
        "AcquisitionFrameRateEnable", true);
    __SetNodeValue<double>("AcquisitionFrameRate",
      __GetNodeMax<double>("AcquisitionFrameRate")); 
  }
  __SetNodeValue<bool>("PtpEnable", flag);
}

void LucidCamera::SetAcquisitionStartMode(const int mode) {
  // qDebug() << "SET ACQUISITION START MODE TO " << mode;
  switch (mode) {
  case 0: __SetNodeValue<GenICam::gcstring>("AcquisitionStartMode", "Normal"); 
    break;
  case 1: __SetNodeValue<GenICam::gcstring>("AcquisitionStartMode", "LowLatency");
    break;
  case 2: __SetNodeValue<GenICam::gcstring>("AcquisitionStartMode", "PTPSync");
    break;
  default: 
    qCritical() << "Acquisition Start Mode " << mode << "is not defined.";
    return;
  }
  // qDebug() << " > Result=" << GetAcquisitionStartMode();
}

void LucidCamera::SetStreamChannelPacketDelay(const int value) {
    // qDebug() << "SetStreamChannelPacketDelay to " << value;
    __SetNodeValue<int64_t>("GevSCPD", 
      static_cast<int64_t>(value));
}

void LucidCamera::SetStreamChannelFrameTransmissionDelay(const int value) {
    // qDebug() << "SetStreamChannelFrameTransmissionDelay to " << value;
    __SetNodeValue<int64_t>("GevSCFTD", 
      static_cast<int64_t>(value));
}

void LucidCamera::SetStreamChannelPacketSize(const int value) {
    __SetNodeValue<int64_t>(
      "GevSCPSPacketSize", value);
}

void LucidCamera::SetPixelFormat(const int format) {
  if (isRunning()) {
    qCritical() << "Cannot change pixel format in streaming mode.";
    return;
  }

  switch (format) {
  case PIXELFORMAT_MONO8:
    __SetNodeValue<GenICam::gcstring>("PixelFormat", "Mono8");
    break;
  case PIXELFORMAT_MONO12:
    __SetNodeValue<GenICam::gcstring>("PixelFormat", "Mono12");
    break;
  }

  GenICam::gcstring value = __GetNodeValue<GenICam::gcstring>("PixelFormat");
  if (value == "Mono8") {
    pixel_format_ = PIXELFORMAT_MONO8;
  }
  else if (value == "Mono12") {
    pixel_format_ = PIXELFORMAT_MONO12;
  }
}

void LucidCamera::SetVoltageExternalEnabled(bool enabled) {
  // Check line4 -> VoltageExternalEnable == true
  {
    std::lock_guard<std::mutex> lock(line_selector_mutex_);
    __SetLineSelector(LINESELECTOR_LINE4);

    /*
    const int prev_enabled = __GetNodeValue<bool>("VoltageExternalEnable");
    if (enabled && prev_enabled) {
      return;
    }
    else if (!enabled && !prev_enabled) {
      return;
    }
    */
    __SetNodeValue<bool>("VoltageExternalEnable", enabled);
  }
  emit voltageExternalEnabledChanged(enabled);
}

void LucidCamera::SetOptoStrobeOutputEnabled(bool enabled) {
  // Check line1 -> LineSource == ExposureActive
  {
    std::lock_guard<std::mutex> lock(line_selector_mutex_);
    __SetLineSelector(LINESELECTOR_LINE1);

    const int mode = __GetLineMode();
    if (enabled && mode != LINEMODE_OUTPUT) {
      __SetLineMode(LINEMODE_OUTPUT);
    }
    /*
    const int source = __GetLineSource();
    if (enabled && source == LINESOURCE_EXPOSUREACTIVE) {
      return;
    }
    else if (!enabled && source == LINESOURCE_OFF) {
      return;
    }
    */
    __SetLineSource(enabled ? LINESOURCE_EXPOSUREACTIVE : LINESOURCE_OFF);
  }
  emit optoStrobeOutputEnabledChanged(enabled);
}

void LucidCamera::SetIsValid(bool valid) {
  if (is_valid_ == valid) return;
  is_valid_ = valid;

  emit exposureRangeChanged(GetMinExposure(), GetMaxExposure());
  emit gainRangeChanged(GetMinGain(), GetMaxGain());
  emit fpsRangeChanged(GetMinFrameRate(), GetMaxFrameRate());

  emit exposureChanged(GetExposure());
  emit gainChanged(GetGain());
  emit fpsChanged(GetFrameRate());

  emit roiWidthChanged(GetWidth());
  emit roiHeightChanged(GetHeight());

  emit streamingChanged(is_streaming_);
  emit triggerModeChanged(is_trigger_mode_);
  emit triggerSourceChanged(GetTriggerSource());
  emit triggerActivationChanged(GetTriggerActivation());
  emit userNameChanged(GetUserName());
  emit physicalPixelSizeChanged(GetPhysicalPixelSize());

  emit connectedChanged(valid);
}
bool LucidCamera::GetIsValid() {
  return is_valid_;
}

std::string LucidCamera::ReadFromFile(const std::string& file_name) {
  std::string ret;

  // Get device nodemap
  GenApi::INodeMap* pNodeMap = device_->GetNodeMap();

  // IDevFileStream from GenApi for file reading from file_name
  GenApi::IDevFileStream file;

  // Chose file_name
  GenApi::CEnumerationPtr pFileSelectorNodeNode = pNodeMap->GetNode("FileSelector");
  GenApi::CValuePtr(pFileSelectorNodeNode)->FromString(file_name.c_str());

  std::cout << std::endl;
  std::cout << "Checking FileOpenMode" << std::endl;
  GenApi::CEnumerationPtr pFileOpenModeNode = pNodeMap->GetNode("FileOpenMode");

  // Check if FileOpenMode is Read-Only. If it is Read-Only, file_name was not properly closed.
  GenApi::EAccessMode fileOpenModeNodeAccessMode = pFileOpenModeNode->GetAccessMode();
  if (fileOpenModeNodeAccessMode == GenApi::EAccessMode::RO)
  {
    // Execute the Close command to close the file_name on the device
    std::cout << "Setting FileOperationSelector to Close" << std::endl;
    GenApi::CEnumerationPtr pFileOperationSelectorNode = pNodeMap->GetNode("FileOperationSelector");
    GenApi::CValuePtr(pFileOperationSelectorNode)->FromString("Close");

    std::cout << "Executing FileOperationExecute to close " << file_name.c_str() << std::endl;
    GenApi::CCommandPtr pFileOperationExecuteCommand = pNodeMap->GetNode("FileOperationExecute");
    pFileOperationExecuteCommand->Execute();
  }

  // Open file_name for reading
  std::cout << "Attempting to open " << file_name.c_str() << " for reading." << std::endl;
  file.open(pNodeMap, file_name.c_str(), std::ios_base::in | std::ios_base::binary);

  if (!file.fail())
  {
    // Check FileSize, the size of the file stored on file_name
    std::cout << "Retrieving FileSize on " << file_name.c_str() << "..." << std::endl;
    GenApi::CIntegerPtr pFileSizeNode = pNodeMap->GetNode("FileSize");
    size_t fileSize = pFileSizeNode->GetValue();

    std::cout << "FileSize is: " << fileSize << " byte";
    if (fileSize > 1 || fileSize == 0)
    {
      // Use plural when necessary
      std::cout << "s";
    }
    std::cout << std::endl;

    std::cout << "Displaying contents of " << file_name.c_str() << ":" << std::endl;
    std::cout << file.rdbuf() << std::endl;

    std::ostringstream ss;
    ss << file.rdbuf();
    ret = ss.str();
  }

  // Close file_name
  file.close();

  return ret;
}

bool LucidCamera::WriteToFile(const std::string& file_name, const std::string& buf) {
  // Get device nodemap
  GenApi::INodeMap* pNodeMap = device_->GetNodeMap();

  // ODevFileStream from GenApi for file writing to file_name
  GenApi::ODevFileStream file;

  // Check size of input file
  std::cout << "Input string '" << buf.c_str() << "' is " << buf.length() << " characters" << std::endl;

  // Choose file_name
  GenApi::CEnumerationPtr pFileSelectorNode = pNodeMap->GetNode("FileSelector");
  GenApi::CValuePtr(pFileSelectorNode)->FromString(file_name.c_str());

  GenApi::CEnumerationPtr pFileOperationSelectorNode = pNodeMap->GetNode("FileOperationSelector");
  GenApi::CCommandPtr pFileOperationExecuteCommand = pNodeMap->GetNode("FileOperationExecute");

  // Check if FileOpenMode is Read-Only. If it is Read-Only, file_name was not properly closed.
  std::cout << "Checking FileOpenMode" << std::endl;
  GenApi::CEnumerationPtr pFileOpenModeNode = pNodeMap->GetNode("FileOpenMode");

  GenApi::EAccessMode fileOpenModeNodeAccessMode = pFileOpenModeNode->GetAccessMode();
  if (fileOpenModeNodeAccessMode == GenApi::EAccessMode::RO)
  {
    // Execute the Close command to close the file_name on the device
    std::cout << "Setting FileOperationSelector to Close" << std::endl;
    GenApi::CValuePtr(pFileOperationSelectorNode)->FromString("Close");

    std::cout << "Executing FileOperationExecute to close " << file_name.c_str() << std::endl;
    pFileOperationExecuteCommand->Execute();
  }

  // Set FileOpenMode to Write and Open/Close to clear file_name
  // This is also used to confirm the number in FileStorageSize once file_name is cleared
  std::cout << "Setting FileOpenMode to Write" << std::endl;
  GenApi::CValuePtr(pFileOpenModeNode)->FromString("Write");

  // If FileOperationSelector == Open, need to call FileOperationExit twice to close the file_name again
  if (pFileOperationSelectorNode->GetCurrentEntry()->GetSymbolic() == "Open")
  {
    pFileOperationExecuteCommand->Execute();
  }
  pFileOperationExecuteCommand->Execute();

  std::cout << "Attempting to open " << file_name.c_str() << " for writing." << std::endl;
  file.open(pNodeMap, file_name.c_str());

  bool success = false;
  if (!file.fail())
  {
    std::cout << "Writing '" << buf.c_str() << "' to " << file_name.c_str() << " on device..." << std::endl;
    file.write(buf.c_str(), buf.length());
    success = true;
  }

  // Close file_name
  file.close();

  return success;
}

int LucidCamera::__GetLineSelector() {
  GenICam::gcstring str
    = __GetNodeValue<GenICam::gcstring>(
      "LineSelector");
  if (str == "Line0") return LINESELECTOR_LINE0;
  else if (str == "Line1") return LINESELECTOR_LINE1;
  else if (str == "Line2") return LINESELECTOR_LINE2;
  else if (str == "Line3") return LINESELECTOR_LINE3;
  else if (str == "Line4") return LINESELECTOR_LINE4;
  else return LINESELECTOR_UNDEFINED;
}

int LucidCamera::__GetLineMode() {
  GenICam::gcstring str
    = __GetNodeValue<GenICam::gcstring>(
      "LineMode");
  if (str == "Input") return LINEMODE_INPUT;
  else if (str == "Output") return LINEMODE_OUTPUT;
  else return LINEMODE_UNDEFINED;
}

int LucidCamera::__GetLineSource() {
  GenICam::gcstring str
    = __GetNodeValue<GenICam::gcstring>(
      "LineSource");
  if (str == "Off") return LINESOURCE_OFF;
  else if (str == "ExposureActive") return LINESOURCE_EXPOSUREACTIVE;
  else if (str == "UserOutput0") return LINESOURCE_USEROUTPUT0;
  else if (str == "UserOutput1") return LINESOURCE_USEROUTPUT1;
  else if (str == "UserOutput2") return LINESOURCE_USEROUTPUT2;
  else if (str == "UserOutput3") return LINESOURCE_USEROUTPUT3;
  else if (str == "SerialTransmit") return LINESOURCE_SERIALTRANSMIT;
  else if (str == "Timer0Active") return LINESOURCE_TIMER0ACTIVE;
  else if (str == "Counter0Active") return LINESOURCE_COUNTER0ACTIVE;
  else if (str == "Timer1Active") return LINESOURCE_TIMER1ACTIVE;
  else if (str == "Counter1Active") return LINESOURCE_COUNTER1ACTIVE;
  else return LINESOURCE_OFF;
}

void LucidCamera::__SetLineSelector(int line_selector_type) {
  if (__GetLineSelector() == line_selector_type) return;
  std::string str;
  switch (line_selector_type) {
  case LINESELECTOR_LINE0: str = "Line0"; break;
  case LINESELECTOR_LINE1: str = "Line1"; break;
  case LINESELECTOR_LINE2: str = "Line2"; break;
  case LINESELECTOR_LINE3: str = "Line3"; break;
  case LINESELECTOR_LINE4: str = "Line4"; break;
  default: str = "Line0"; break;
  }
  __SetNodeValue<GenICam::gcstring>(
    "LineSelector",
    str.c_str()
  );
}

void LucidCamera::__SetLineMode(int line_mode_type) {
  if (__GetLineMode() == line_mode_type) return;
  std::string str;
  switch (line_mode_type) {
  case LINEMODE_INPUT: str = "Input"; break;
  case LINEMODE_OUTPUT: str = "Output"; break;
  default: str = "Output"; break;
  }
  __SetNodeValue<GenICam::gcstring>(
    "LineMode",
    str.c_str()
  );
}

void LucidCamera::__SetLineSource(int line_source_type) {
  if (__GetLineSource() == line_source_type) return;
  std::string str;
  switch (line_source_type) {
  case LINESOURCE_OFF: str = "Off"; break;
  case LINESOURCE_EXPOSUREACTIVE: str = "ExposureActive"; break;
  case LINESOURCE_USEROUTPUT0: str = "UserOutput0"; break;
  case LINESOURCE_USEROUTPUT1: str = "UserOutput1"; break;
  case LINESOURCE_USEROUTPUT2: str = "UserOutput2"; break;
  case LINESOURCE_USEROUTPUT3: str = "UserOutput3"; break;
  case LINESOURCE_SERIALTRANSMIT: str = "SerialTransmit"; break;
  case LINESOURCE_TIMER0ACTIVE: str = "Timer0Active"; break;
  case LINESOURCE_COUNTER0ACTIVE: str = "Counter0Active"; break;
  case LINESOURCE_TIMER1ACTIVE: str = "Timer1Active"; break;
  case LINESOURCE_COUNTER1ACTIVE: str = "Counter1Active"; break;
  default: str = "Off"; break;
  }
  __SetNodeValue<GenICam::gcstring>(
    "LineSource",
    str.c_str()
  );
}

// Image timeout
//    Timeout for grabbing images (in milliseconds). If no image is available at
//    the end of the timeout, an exception is thrown. The timeout is the maximum
//    time to wait for an image; however, getting an image will return as soon as
//    an image is available, not waiting the full extent of the timeout.
#define LUCID_TIMEOUT 5000 // ms

void LucidCamera::run() {
  emit beforeStreamingStarted();
  emit streamingChanged(true);
  
  const bool ptp_enabled = GetPtpEnable();
  const std::string ptp_status
    = __GetNodeValue<GenICam::gcstring>("PtpStatus").c_str();
  
  const int64_t link_throughput_reserve
    = __GetNodeValue<int64_t>("DeviceLinkThroughputReserve");

  const int acquisition_start_mode = GetAcquisitionStartMode();
  const double acquisition_frame_rate
    = __GetNodeValue<double>("AcquisitionFrameRate");
  double ptp_sync_frame_rate = -1;
  if (ptp_enabled) {
    ptp_sync_frame_rate = __GetNodeValue<double>("PTPSyncFrameRate");
  }

  const int packet_size = GetStreamChannelPacketSize();
  const int scpd = GetStreamChannelPacketDelay();
  const int scftd = GetStreamChannelFrameTransmissionDelay();
  
  const bool packet_resend_enable
    = Arena::GetNodeValue<bool>(
      device_->GetTLStreamNodeMap(),
      "StreamPacketResendEnable");
  const bool auto_negotiate_packet_size
    = Arena::GetNodeValue<bool>(
      device_->GetTLStreamNodeMap(),
      "StreamAutoNegotiatePacketSize");
  
  QString msg = "\n[Start Stream - " + GetUserName() + "]" +
    " \n PTP enable=" + (ptp_enabled ? "true" : "false") +
    " \n PTP status=" + QString::fromStdString(ptp_status) +
    " \n Acquisition start mode=" + QString::number(acquisition_start_mode) +
    " \n Acquisition frame rate=" + QString::number(acquisition_frame_rate) +
    " \n PTP sync frame rate=" + QString::number(ptp_sync_frame_rate) +
    " \n Trigger mode=" + (GetTriggerMode() ? "true" : "false") +
    " \n Packet resend enable=" + (packet_resend_enable ? "true" : "false") +
    " \n Auto negotiate packet size=" + (auto_negotiate_packet_size ? "true" : "false") +
    " \n Packet size=" + QString::number(packet_size) +
    " \n Stream channel packet delay=" + QString::number(scpd) +
    " \n Stream channel frame transmission delay=" + QString::number(scftd) +
    " \n Link throughput reserve=" + QString::number(link_throughput_reserve) +
    " \n";
  qInfo().noquote() << msg;

  Arena::IImage* raw;

  while (is_streaming_) {
    //qDebug() << "camera run";
    try {
      if (!is_valid_) {
        qCritical() << "Camera run error: Object has disconnected.";
        is_streaming_ = false;
        emit beforeStreamingFinished();
        emit streamingFailed(); // failed to get frame
        emit streamingChanged(false);
        return;
      }
      /// *pImage and MonoFrame share same memory.
      //qDebug() << "LUCID_CAMERA wait for next image";
      raw = device_->GetImage(LUCID_TIMEOUT);
      cv::Mat mono = cv::Mat((int)raw->GetHeight(),
        (int)raw->GetWidth(),
        CV_8UC1,
        (void*)raw->GetData());
      //qDebug() << "MONO MEMORY =" << mono.data;
      //qDebug() << " > LUCID_CAMERA FRAME GRABBED!";
      if (!pass_grabbing_) emit frameGrabbed(mono); // This emitter must be dealt immediately
      device_->RequeueBuffer(raw);
    }
    catch (GenICam::GenericException& err) {
      if (!device_->IsConnected()) {
        qCritical() << "Camera streaming failed: Device connection lost";
        is_streaming_ = false;
        SetIsValid(false);
        emit deviceLostExceptionThrown("");
      }
      else {
        qCritical() << "Acquisition image failed: " << err.what();
        // @raw is only received when no exception, so no need to RequeueBuffer in here.
      }
    }
  }
  emit beforeStreamingFinished();
  try {
    device_->StopStream();
  }
  catch (GenICam::GenericException& ge) {
  }
  emit afterStreamingFinished();
  emit streamingChanged(false);
}

} // namespace qlucidcamera