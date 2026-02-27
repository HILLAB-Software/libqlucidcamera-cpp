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
  
#ifndef LIBQLUCIDCAMERA_LUCID_CAMERA_H
#define LIBQLUCIDCAMERA_LUCID_CAMERA_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <string>
#include <queue>

namespace Arena {
class IDevice;
}

namespace qlucidcamera {

class LucidCamera : public QThread {
  Q_OBJECT

  Q_PROPERTY(bool connected READ GetIsValid WRITE SetIsValid NOTIFY connectedChanged)
  Q_PROPERTY(QString userName READ GetUserName WRITE SetUserName NOTIFY userNameChanged)
  Q_PROPERTY(QString serialNumber READ GetSerialNumber NOTIFY connectedChanged)
  Q_PROPERTY(QString modelName READ GetModelName NOTIFY connectedChanged)
  Q_PROPERTY(double exposure READ GetExposure WRITE SetExposure NOTIFY exposureChanged)
  Q_PROPERTY(double gain READ GetGain WRITE SetGain NOTIFY gainChanged)
  Q_PROPERTY(double fps READ GetFrameRate WRITE SetFrameRate NOTIFY fpsChanged)
  Q_PROPERTY(bool   triggerMode     READ GetTriggerMode WRITE SetTriggerMode NOTIFY triggerModeChanged)
  Q_PROPERTY(int    triggerSource READ GetTriggerSource WRITE SetTriggerSource NOTIFY triggerSourceChanged)
  Q_PROPERTY(int    triggerActivation READ GetTriggerActivation WRITE SetTriggerActivation NOTIFY triggerActivationChanged)
  Q_PROPERTY(int    pixelFormat READ GetPixelFormat WRITE SetPixelFormat NOTIFY pixelFormatChanged)
  Q_PROPERTY(double physicalPixelSize READ GetPhysicalPixelSize NOTIFY physicalPixelSizeChanged)
  Q_PROPERTY(bool   voltageExternalEnabled READ GetVoltageExternalEnabled WRITE SetVoltageExternalEnabled NOTIFY voltageExternalEnabledChanged)
  Q_PROPERTY(bool   optoStrobeOutputEnabled READ GetOptoStrobeOutputEnabled WRITE SetOptoStrobeOutputEnabled NOTIFY optoStrobeOutputEnabledChanged)

  Q_PROPERTY(double exposureMin       READ GetMinExposure     NOTIFY exposureRangeChanged)
  Q_PROPERTY(double exposureMax       READ GetMaxExposure     NOTIFY exposureRangeChanged)
  Q_PROPERTY(double gainMin           READ GetMinGain         NOTIFY gainRangeChanged)
  Q_PROPERTY(double gainMax           READ GetMaxGain         NOTIFY gainRangeChanged)
  Q_PROPERTY(double fpsMin            READ GetMinFrameRate    NOTIFY fpsRangeChanged)
  Q_PROPERTY(double fpsMax            READ GetMaxFrameRate    NOTIFY fpsRangeChanged)
  Q_PROPERTY(int    roiWidth          READ GetWidth           NOTIFY roiWidthChanged)
  Q_PROPERTY(int    roiHeight         READ GetHeight          NOTIFY roiHeightChanged)
  Q_PROPERTY(bool   isStreaming       READ GetIsStreaming     NOTIFY streamingChanged)

public:
  enum TriggerSource {
  TRIGGERSOURCE_UNDEFINED = -1,
  TRIGGERSOURCE_SOFTWARE = 0, 
  TRIGGERSOURCE_LINE0 = 1, 
  TRIGGERSOURCE_LINE2 = 3,
  TRIGGERSOURCE_LINE3 = 4, 
  TRIGGERSOURCE_ACTION0 = 10
  };
  Q_ENUM(TriggerSource)

  enum TriggerActivation {
  TRIGGERACTIVATION_UNDEFINED = -1,
  TRIGGERACTIVATION_RISINGEDGE = 0, 
  TRIGGERACTIVATION_FALLINGEDGE = 1, 
  TRIGGERACTIVATION_ANYEDGE = 2, 
  TRIGGERACTIVATION_LEVELHIGH = 3, 
  TRIGGERACTIVATION_LEVELLOW = 4
  };
  Q_ENUM(TriggerActivation)

  enum LineSelector {
  LINESELECTOR_UNDEFINED = -1, 
  LINESELECTOR_LINE0 = 0,
  LINESELECTOR_LINE1 = 1,
  LINESELECTOR_LINE2 = 2,
  LINESELECTOR_LINE3 = 3,
  LINESELECTOR_LINE4 = 4
  };
  Q_ENUM(LineSelector)

  enum LineMode {
  LINEMODE_UNDEFINED = -1, 
  LINEMODE_INPUT = 0, 
  LINEMODE_OUTPUT = 1
  };
  Q_ENUM(LineMode)

  enum LineSource {
  LINESOURCE_OFF = 0, 
  LINESOURCE_EXPOSUREACTIVE = 4, 
  LINESOURCE_USEROUTPUT0 = 5,
  LINESOURCE_USEROUTPUT1 = 6,
  LINESOURCE_USEROUTPUT2 = 7,
  LINESOURCE_USEROUTPUT3 = 8,
  LINESOURCE_SERIALTRANSMIT = 9, 
  LINESOURCE_TIMER0ACTIVE = 13, 
  LINESOURCE_COUNTER0ACTIVE = 14, 
  LINESOURCE_TIMER1ACTIVE = 15, 
  LINESOURCE_COUNTER1ACTIVE = 16
  };

  enum AcquisitionStartMode {
  ACQUISITIONSTARTMODE_NORMAL = 0, 
  ACQUISITIONSTARTMODE_LOWLATENCY = 1,
  ACQUISITIONSTARTMODE_PTPSYNC = 2
  };

  enum PixelFormat {
  PIXELFORMAT_MONO8 = 0, 
  PIXELFORMAT_MONO12 = 1
  };

  explicit LucidCamera(QObject* parent = nullptr);
  ~LucidCamera();
  void SetDevice(Arena::IDevice* created_device);

Q_SIGNALS:
  void exposureChanged(double exposure);
  void gainChanged(double gain);
  void fpsChanged(double fps);
  void exposureRangeChanged(double min, double max);
  void gainRangeChanged(double min, double max);
  void fpsRangeChanged(double min, double max);
  void roiWidthChanged(int value);
  void roiHeightChanged(int value);
  void streamingChanged(bool streaming);
  void triggerModeChanged(bool mode);
  void triggerSourceChanged(int source);
  void triggerActivationChanged(int activation);

  void frameGrabbed(cv::Mat frame);
  void streamingFailed();
  void beforeStreamingStarted();
  void beforeStreamingFinished();
  void afterStreamingFinished();

  void deviceLostExceptionThrown(QString);

  void userNameChanged(QString user_name);
  void connectedChanged(bool connected);
  void pixelFormatChanged(PixelFormat format);
  void physicalPixelSizeChanged(double pixel_size);
  void voltageExternalEnabledChanged(bool enabled);
  void optoStrobeOutputEnabledChanged(bool enabled);

public Q_SLOTS:
  Arena::IDevice* GetDevice();
  QString GetModelName();
  QString GetSerialNumber();
  QString GetMacAddress();
  QString GetUserName();
  double GetMaxExposure();
  double GetMinExposure();
  double GetMaxGain();
  double GetMinGain();
  double GetExposure();
  double GetGain();
  double GetMinFrameRate();
  double GetMaxFrameRate();
  double GetFrameRate();
  int GetWidth();
  int GetMaxWidth();
  int GetMinWidth();
  int GetHeight();
  int GetMaxHeight();
  int GetMinHeight();
  int GetOffsetX();
  int GetMaxOffsetX();
  int GetOffsetY();
  int GetMaxOffsetY();
  bool GetIsStreaming();
  bool GetTriggerMode();
  int GetTriggerSource();
  int GetTriggerActivation();
  bool GetPtpEnable();
  QString GetPtpStatus();
  int GetAcquisitionStartMode();
  int GetStreamChannelPacketDelay();
  int GetStreamChannelFrameTransmissionDelay();
  int GetStreamChannelPacketSize();
  int GetPixelFormat();
  double GetPhysicalPixelSize();
  bool GetVoltageExternalEnabled();
  bool GetOptoStrobeOutputEnabled();

public Q_SLOTS:
  Q_INVOKABLE bool SetStreaming(const bool flag);
  Q_INVOKABLE bool SetTriggerMode(const bool flag);
  Q_INVOKABLE void SoftwareTrigger();
  Q_INVOKABLE void DeviceReset(); // It will lost connection
  void SetUserName(const QString value);
  void SetTriggerSource(int trigger_source_type);
  void SetTriggerActivation(int trigger_activation_type);
  void ZoomRoi(const int zoom_factor);
  void SetExposure(const double Value);
  void SetGain(const double Value);
  void SetFrameRate(const double Value);
  void SetWidth(const int Value);
  void SetHeight(const int Value);
  void SetOffsetX(const int Value);
  void SetOffsetY(const int Value);
  void SetPtpEnable(const bool flag);
  void SetAcquisitionStartMode(const int mode);
  void SetStreamChannelPacketDelay(const int value);
  void SetStreamChannelFrameTransmissionDelay(const int value);
  void SetStreamChannelPacketSize(const int value);
  void SetPixelFormat(const int format);
  void SetVoltageExternalEnabled(bool enabled);
  void SetOptoStrobeOutputEnabled(bool enabled);

public:
  void SetIsValid(bool valid);
  bool GetIsValid();
  std::string ReadFromFile(const std::string& file_name);
  bool WriteToFile(const std::string& file_name, const std::string& buf);
  void StartPassGrabbing() { pass_grabbing_ = true; }
  void StopPassGrabbing() { pass_grabbing_ = false; }

private:
  int __GetLineMode();
  int __GetLineSource();
  int __GetLineSelector();
  void __SetLineSelector(int line_selector_type);
  void __SetLineMode(int line_mode_type);
  void __SetLineSource(int line_source_type);
  void run() override;

  template <typename T>
  T __GetNodeValue(const std::string& name);

  template <typename T>
  T __GetNodeMax(const std::string& name);

  template <typename T>
  T __GetNodeMin(const std::string& name);

  template <typename T>
  void __SetNodeValue(const std::string& name, const T& value);

  void __ExecuteNode(const std::string& name);


  //catch (GenICam::GenericException& ge) {
  //  qCritical() << ge.what();
  //}

private:
  bool is_valid_ = false;
  Arena::IDevice* device_ = nullptr;
  bool is_streaming_ = false;
  bool is_trigger_mode_ = false;
  int pixel_format_ = PIXELFORMAT_MONO8;
  bool pass_grabbing_ = false;
  std::mutex line_selector_mutex_;

  /*
private:
  GenApi::CFloatPtr gen_exposure_;
  GenApi::CFloatPtr gen_gain_;
  GenApi::CFloatPtr gen_fps_;
  GenApi::CFloatPtr gen_ptp_fps_;
  GenApi::CIntegerPtr gen_roi_width_;
  GenApi::CIntegerPtr gen_roi_height_;
  GenApi::CIntegerPtr gen_offset_x_;
  GenApi::CIntegerPtr gen_offset_y_;
  */


  //gen_exposure_ = device_->GetNodeMap()->GetNode("ExposureTime");
  //gen_gain_ = device_->GetNodeMap()->GetNode("Gain");
  //gen_fps_ = device_->GetNodeMap()->GetNode("AcquisitionFrameRate");
  //gen_ptp_fps_ = device_->GetNodeMap()->GetNode("PTPSyncFrameRate");
  //gen_roi_width_ = device_->GetNodeMap()->GetNode("Width");
  //gen_roi_height_ = device_->GetNodeMap()->GetNode("Height");
  //gen_offset_x_ = device_->GetNodeMap()->GetNode("OffsetX");
  //gen_offset_y_ = device_->GetNodeMap()->GetNode("OffsetY");

friend class LucidCameraConnector;
};

} // namespace qlucidcamera

#endif // LIBQLUCIDCAMERA_LUCID_CAMERA_H