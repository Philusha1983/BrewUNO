#include <BrewUNO/ActiveStatus.h>

ActiveStatus::ActiveStatus(FS *fs) : _fs(fs)
{
}

ActiveStatus::~ActiveStatus() {}

boolean ActiveStatus::LoadActiveStatusSettings()
{
    File configFile = _fs->open(ACTIVE_STATUS_FILE, "r");
    if (configFile)
    {
        size_t size = configFile.size();
        if (size <= MAX_ACTIVESTATUS_SIZE)
        {
            DynamicJsonDocument _activeStatusJsonDocument = DynamicJsonDocument(MAX_ACTIVESTATUS_SIZE);
            DeserializationError error = deserializeJson(_activeStatusJsonDocument, configFile);
            if (error == DeserializationError::Ok && _activeStatusJsonDocument.is<JsonObject>())
            {
                JsonObject _activeStatus = _activeStatusJsonDocument.as<JsonObject>();
                ActiveStep = _activeStatus["active_step"];
                ActiveMashStepIndex = _activeStatus["active_mash_step_index"];
                ActiveBoilStepIndex = _activeStatus["active_boil_step_index"] | "";
                BoilTime = _activeStatus["boil_time"];
                BoilTargetTemperature = _activeStatus["boil_target_temperature"];
                TargetTemperature = _activeStatus["target_temperature"];
                EndTime = _activeStatus["end_time"];
                StartTime = _activeStatus["start_time"];
                TimeNow = _activeStatus["time_now"];
                BrewStarted = _activeStatus["brew_started"];
                PWM = _activeStatus["pwm"];
                Recirculation = _activeStatus["recirculation"];
                BoilPowerPercentage = _activeStatus["boil_power_percentage"];
                PIDTuning = _activeStatus["pid_tuning"];
                configFile.close();
            }
        }
        configFile.close();
    }

    return true;
}

String ActiveStatus::GetJson()
{
    Serial.println("GET Json:");
    String status = "{\"active_step\":" + String(ActiveStep) + "," +
                    "\"active_mash_step_index\":" + String(ActiveMashStepIndex) + "," +
                    "\"active_boil_step_index\":\"" + String(ActiveBoilStepIndex) + "\"" + "," +
                    "\"boil_time\":" + String(BoilTime) + "," +
                    "\"boil_target_temperature\":" + String(BoilTargetTemperature) + "," +
                    "\"target_temperature\":" + String(TargetTemperature) + "," +
                    "\"start_time\":" + String(StartTime) + "," +
                    "\"end_time\":" + String(EndTime) + "," +
                    "\"time_now\":" + String(TimeNow) + "," +
                    "\"brew_started\":" + String(BrewStarted) + "," +
                    "\"temperature\":" + String(Temperature) + "," +
                    "\"pwm\":" + String(PWM) + ',' +
                    "\"recirculation\":" + String(Recirculation) + "," +
                    "\"pid_tuning\":" + String(PIDTuning) + "," +
                    "\"boil_power_percentage\":" + String(BoilPowerPercentage) +
                    "}";
    Serial.println(status);
    return status;
}

void ActiveStatus::SaveActiveStatus(time_t startTime,
                                    time_t endTime,
                                    time_t timeNow,
                                    float targetTemperature,
                                    int activeMashStepIndex,
                                    String activeBoilStepIndex,
                                    int boilTime,
                                    float boilTargetTemperature,
                                    int activeStep,
                                    boolean brewStarted)
{
    ActiveStep = activeStep;

    ActiveMashStepIndex = activeMashStepIndex;

    ActiveBoilStepIndex = activeBoilStepIndex;
    BoilTime = boilTime;
    BoilTargetTemperature = boilTargetTemperature;
    TargetTemperature = targetTemperature;
    StartTime = startTime;
    EndTime = endTime;
    TimeNow = timeNow;
    BrewStarted = brewStarted;
    Temperature = 0;
    PWM = 0;
    Recirculation = false;
    PIDTuning = false;
    BoilPowerPercentage = 0;

    SaveActiveStatus();
}

time_t lastWrite = now();

void ActiveStatus::SaveActiveStatusLoop()
{
    if ((!BrewStarted) || (now() - lastWrite < 60))
        return;

    SaveActiveStatus();
}

void ActiveStatus::SaveActiveStatus()
{
    lastWrite = now();

    DynamicJsonDocument _activeStatusJsonDocument = DynamicJsonDocument(MAX_ACTIVESTATUS_SIZE);
    JsonObject _activeStatus = _activeStatusJsonDocument.to<JsonObject>();

    _activeStatus["active_step"] = ActiveStep;
    _activeStatus["active_mash_step_index"] = ActiveMashStepIndex;
    _activeStatus["active_boil_step_index"] = ActiveBoilStepIndex;
    _activeStatus["boil_time"] = BoilTime;
    _activeStatus["boil_target_temperature"] = BoilTargetTemperature;
    _activeStatus["target_temperature"] = TargetTemperature;
    _activeStatus["start_time"] = StartTime;
    _activeStatus["end_time"] = EndTime;
    _activeStatus["time_now"] = now();
    _activeStatus["brew_started"] = BrewStarted;
    _activeStatus["temperature"] = Temperature;
    _activeStatus["pwm"] = PWM;
    _activeStatus["recirculation"] = Recirculation;
    _activeStatus["boil_power_percentage"] = BoilPowerPercentage;
    _activeStatus["pid_tuning"] = PIDTuning;

    File configFile = _fs->open(ACTIVE_STATUS_FILE, "w");
    if (configFile)
        serializeJson(_activeStatus, configFile);

    configFile.close();
}

float RawHigh = 99.3;
float RawLow = 0.3;
float ReferenceHigh = 99.9;
float ReferenceLow = 0;
float RawRange = RawHigh - RawLow;
float ReferenceRange = ReferenceHigh - ReferenceLow;

void ActiveStatus::SetTemperature(float temperature)
{
    if (temperature > 0 && temperature >= Temperature - 10)
    {
        float CorrectedValue = (((temperature - RawLow) * ReferenceRange) / RawRange) + ReferenceLow;
        Temperature = CorrectedValue;
    }
}