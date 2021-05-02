#pragma once

#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include <string>
#include <vector>

struct SDLGamepadState{
    // Axis values range from -1.0f to 1.0f
    struct LeftStickAxis {float x= 0; float y = 0;} LeftStick;
    // Axis values range from -1.0f - 1.0f
    struct RightStickAxis {float x = 0; float y = 0;} RightStick;
    int A = 0;
    int B = 0;
    int X = 0;
    int Y = 0;
    int DPadUp = 0;
    int DPadDown = 0;
    int DPadLeft = 0;
    int DPadRight = 0;
    int LeftShoulder = 0;
    int RightShoulder = 0;
    int LeftStickClick = 0;
    int RightStickClick = 0;
    int Start = 0;
    int Back = 0;
    int Touchpad = 0;
    int Guide = 0;
    int Misc = 0;
    int Paddle1 = 0;
    int Paddle2 = 0;
    int Paddle3 = 0;
    int Paddle4 = 0;
    // Axis values range from 0.0f to 1.0f
    float LeftTrigger = 0.0f;
    // Axis values range from 0.0f to 1.0f
    float RightTrigger = 0.0f;
};

struct SDLGamepadSensorState {
    // Taken from SDL_sensor.h
    // For game controllers held in front of you,
    // the axes are defined as follows:
    // -X ... +X : left ... right
    // -Y ... +Y : bottom ... top
    // -Z ... +Z : farther ... closer

    // values[0]: Acceleration on the x axis
    // values[1]: Acceleration on the y axis
    // values[2]: Acceleration on the z axis
    float Accelerometer[3] = {0.0f, 0.0f, 0.0f};

    // values[0]: Angular speed around the x axis (pitch)
    // values[1]: Angular speed around the y axis (yaw)
    // values[2]: Angular speed around the z axis (roll)
    float Gyroscope[3] = {0.0f, 0.0f, 0.0f};
};

struct SDLGamepadTouchpadFinger{
    Uint8 state;
    float x = 0.0f;
    float y = 0.0f;
    float pressure = 0.0f;
};

struct SDLGamepadTouchpad {
    std::vector<SDLGamepadTouchpadFinger> fingers;
};

class SDLGamepad {
private:
    std::string name = "";
    std::string serialNumber = "";
    SDL_GameController * controller;
    int touchpadCount = 0;
    bool hapticsSupported = false;
    bool triggerHapticsSupported = false;
    bool sensorSupported = false;
    bool sensing = false;
    bool gyroSupported = false;
    bool gyroActive = false;
    bool accelSupported = false;
    bool accelActive = false;
    bool touchpadSupported = false;
    bool queryTouchpads = false;
    bool sensorEnabled = false;

public:
    SDL_JoystickID id;
    SDLGamepadState last_state;
    SDLGamepadState state;
    SDLGamepadSensorState last_sensor_state;
    SDLGamepadSensorState sensor_state;
    std::vector<SDLGamepadTouchpad> touchpads;

    SDLGamepad(int index){
        controller = SDL_GameControllerOpen(index);
        id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
        name = SDL_GameControllerName(controller);
        serialNumber = SDL_GameControllerGetSerial(controller);
        if (SDL_GameControllerRumble(controller, 0, 0, 0) == 0){
            hapticsSupported = true;
        }
        if (SDL_GameControllerRumbleTriggers(controller, 0, 0, 0) == 0){
            triggerHapticsSupported = true;
        }
        if (SDL_GameControllerHasSensor(controller, SDL_SENSOR_ACCEL) || SDL_GameControllerHasSensor(controller, SDL_SENSOR_GYRO)){
            sensorSupported = true;
            if (SDL_GameControllerHasSensor(controller, SDL_SENSOR_ACCEL)){
                accelSupported = true;
            }
            if (SDL_GameControllerHasSensor(controller, SDL_SENSOR_GYRO)){
                gyroSupported = true;
            }
        }
        touchpadCount = SDL_GameControllerGetNumTouchpads(controller);
        if (touchpadCount){
            touchpadSupported = true;
            touchpads.resize(touchpadCount);
            for (int i = 0; i < touchpadCount; i++){
                touchpads[i].fingers.resize(SDL_GameControllerGetNumTouchpadFingers(controller, i));
            }
        }
    }

    ~SDLGamepad(){
        SDL_GameControllerClose(controller);
    }

    std::string getName(){
        return name;
    }

    SDL_GameController * getController(){
        return controller;
    }

    int getTouchpadCount(){
        return touchpadCount;
    }

    bool hasHaptics(){
        return hapticsSupported;
    }

    bool hasTriggerHaptics(){
        return triggerHapticsSupported;
    }

    bool hasSensors(){
        return sensorSupported;
    }

    bool hasAccelerometer(){
        return sensorSupported && accelSupported;
    }

    bool hasGyroscope(){
        return sensorSupported && gyroSupported;
    }

    bool hasAllSensors(){
        return hasAccelerometer() && hasGyroscope();
    }

    void setSensor(SDL_SensorType type, SDL_bool active){
        if (type == SDL_SENSOR_GYRO){
            gyroActive = active;
        }
        if (type == SDL_SENSOR_ACCEL){
            accelActive = active;
        }
        sensorEnabled = (gyroActive || accelActive);
        SDL_GameControllerSetSensorEnabled(controller, type, active);
    }

    void setTouchpadSensing(bool active){
        if (touchpadSupported){
            queryTouchpads = active;
        }
    }

    void pollTouchpad(){
        if (queryTouchpads){
            for (int index = 0; index < touchpadCount; index++){
                for (int finger = 0; finger < touchpads[index].fingers.size(); finger++){
                    SDL_GameControllerGetTouchpadFinger(controller, index, finger,
                     &touchpads[index].fingers[finger].state,
                     &touchpads[index].fingers[finger].x,
                     &touchpads[index].fingers[finger].y,
                     &touchpads[index].fingers[finger].pressure);
                }
            }
        }
    }

    void pollState(){
        last_state = state;
        state = SDLGamepadState();
        last_sensor_state = sensor_state;
        sensor_state = SDLGamepadSensorState();
        //DPad buttons
        state.DPadUp = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
        state.DPadDown = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        state.DPadLeft = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        state.DPadRight = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        //Face Buttons (based on Xbox controller layout)
        state.A = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
        state.B = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B);
        state.X = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X);
        state.Y = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y);
        // Start, Back, and Guide
        state.Start = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START);
        state.Back = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK);
        state.Guide = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_GUIDE);
        //Left Click and Right Click
        state.LeftStickClick = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSTICK);
        state.RightStickClick = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
        //Paddles 1-4
        state.Paddle1 = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_PADDLE1);
        state.Paddle2 = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_PADDLE2);
        state.Paddle3 = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_PADDLE3);
        state.Paddle4 = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_PADDLE4);
        //Touchpad Button and Misc (Xbox Share button, Switch Pro Capture button, and Mic button for PS4/PS5 controllers)
        state.Touchpad = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_TOUCHPAD);
        state.Misc = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_MISC1);
        //Left and Right Shoulder
        state.LeftShoulder = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        state.RightShoulder = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        // Axis values for the left and right stick
        state.LeftStick.x = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / SDL_JOYSTICK_AXIS_MAX;
        state.LeftStick.y = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / SDL_JOYSTICK_AXIS_MAX;
        state.RightStick.x = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) / SDL_JOYSTICK_AXIS_MAX;
        state.RightStick.y = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) / SDL_JOYSTICK_AXIS_MAX;
        //Left and Right Trigger
        state.LeftTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / SDL_JOYSTICK_AXIS_MAX;
        state.RightTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / SDL_JOYSTICK_AXIS_MAX;

        if (sensorEnabled){
            if (accelActive){
                SDL_GameControllerGetSensorData(controller, SDL_SENSOR_ACCEL, sensor_state.Accelerometer, 3);
            }
            if (gyroActive){
                SDL_GameControllerGetSensorData(controller, SDL_SENSOR_GYRO, sensor_state.Gyroscope, 3);
            }
        }  
        pollTouchpad();
    }

    void Rumble(Uint16 left, Uint16 right, Uint32 duration){
        if (hapticsSupported){
            SDL_GameControllerRumble(controller, left, right, duration);
        }
    }

    void RumbleTriggers(Uint16 left_trigger, Uint16 right_trigger, Uint32 duration){
        if (triggerHapticsSupported){
            SDL_GameControllerRumbleTriggers(controller, left_trigger, right_trigger, duration);
        }    
    }

    void SetLED(Uint8 r, Uint8 g, Uint8 b){
        SDL_GameControllerSetLED(controller, r, g, b);
    }
};
