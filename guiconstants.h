// -----------------------------------------------------------------------------
 //    ASPiK Plugin Kernel File:  guiconstants.h
 //
 // -----------------------------------------------------------------------------
#ifndef _guiconstants_h
#define _guiconstants_h

#define _MATH_DEFINES_DEFINED

#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <math.h>

// --- RESERVED PARAMETER ID VALUES
const unsigned int PLUGIN_SIDE_BYPASS = 131072;
const unsigned int XY_TRACKPAD = 131073;
const unsigned int VECTOR_JOYSTICK = 131074;
const unsigned int PRESET_NAME = 131075;
const unsigned int WRITE_PRESET_FILE = 131076;
const unsigned int SCALE_GUI_SIZE = 131077;
// --- 131078 -through- 131999 are RESERVED     ///<RESERVED PARAMETER ID VALUE

// --- custom views may be added using the base here: e.g.
//     const unsigned int CUSTOM_SPECTRUM_VIEW = CUSTOM_VIEW_BASE + 1;
const unsigned int CUSTOM_VIEW_BASE = 132000;

// --- enum for the GUI object's message processing
enum { tinyGUI, verySmallGUI, smallGUI, normalGUI, largeGUI, veryLargeGUI };

inline bool isReservedTag(int tag)
{
    // --- these are reserved
    if (tag >= 131072 && tag <= 131999)
        return true;
    return false;
}

inline bool isBonusParameter(int tag)
{
    // --- these are reserved
    if (tag == XY_TRACKPAD ||
        tag == VECTOR_JOYSTICK ||
        tag == PRESET_NAME ||
        tag == WRITE_PRESET_FILE ||
        tag == SCALE_GUI_SIZE)
        return true;

    return false;
}

// --- typed enumeration helpers
#define enumToInt(ENUM) static_cast<int>(ENUM)

#define compareEnumToInt(ENUM,INT) (static_cast<int>(ENUM) == (INT))

#define compareIntToEnum(INT,ENUM) ((INT) == static_cast<int>(ENUM))

#define convertIntToEnum(INT,ENUM) static_cast<ENUM>(INT)

const double kCTCoefficient = 5.0 / 12.0;

const double kCTCorrFactorZero = pow(10.0, (-1.0 / kCTCoefficient));

const double kCTCorrFactorAnitZero = 1.0 / (1.0 - kCTCorrFactorZero);

const double kCTCorrFactorUnity = 1.0 / (1.0 + kCTCoefficient * log10(1.0 + kCTCorrFactorZero));

const double kCTCorrFactorAntiUnity = 1.0 / (1.0 + (-pow(10.0, (-1.0 / kCTCoefficient))));

const double kCTCorrFactorAntiLog = kCTCoefficient * log10(1.0 + kCTCorrFactorZero);

const double kCTCorrFactorAntiLogScale = 1.0 / (-kCTCoefficient * log10(kCTCorrFactorZero) + kCTCorrFactorAntiLog);

const float kPi = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899f; // changed to float
//const double kPi = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899;

const double kTwoPi = 2.0 * 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// ---
const uint32_t ENVELOPE_DETECT_MODE_PEAK = 0;
const uint32_t ENVELOPE_DETECT_MODE_MS = 1;
const uint32_t ENVELOPE_DETECT_MODE_RMS = 2;
const uint32_t ENVELOPE_DETECT_MODE_NONE = 3;

const float ENVELOPE_DIGITAL_TC = -4.6051701859880913680359829093687;
const float ENVELOPE_ANALOG_TC = -1.0023934309275667804345424248947;

// ---
const float GUI_METER_UPDATE_INTERVAL_MSEC = 50.f;
const float GUI_METER_MIN_DB = -60.f;

#define FLT_EPSILON_PLUS      1.192092896e-07        

#define FLT_EPSILON_MINUS    -1.192092896e-07        

#define FLT_MIN_PLUS          1.175494351e-38        

#define FLT_MIN_MINUS        -1.175494351e-38        

// --- for math.h constants
#define _MATH_DEFINES_DEFINED

enum class smoothingMethod { kLinearSmoother, kLPFSmoother };

enum class taper { kLinearTaper, kLogTaper, kAntiLogTaper, kVoltOctaveTaper };

enum class meterCal { kLinearMeter, kLogMeter };

enum class controlVariableType { kFloat, kDouble, kInt, kTypedEnumStringList, kMeter, kNonVariableBoundControl };


enum class boundVariableType { kFloat, kDouble, kInt, kUInt };

template <class T>
class ParamSmoother
{
public:
    ParamSmoother() { a = 0.0; b = 0.0; z = 0.0; z2 = 0.0; }

    void setSampleRate(T samplingRate)
    {
        sampleRate = samplingRate;

        // --- for LPF smoother
        a = exp(-kTwoPi / (smoothingTimeInMSec * 0.001 * sampleRate));
        b = 1.0 - a;

        // --- for linear smoother
        linInc = (maxVal - minVal) / (smoothingTimeInMSec * 0.001 * sampleRate);
    }

    void initParamSmoother(T smoothingTimeInMs,
        T samplingRate,
        T initValue,
        T minControlValue,
        T maxControlValue,
        smoothingMethod smoother = smoothingMethod::kLPFSmoother)
    {
        minVal = minControlValue;
        maxVal = maxControlValue;
        sampleRate = samplingRate;
        smoothingTimeInMSec = smoothingTimeInMs;

        setSampleRate(samplingRate);

        // --- storage
        z = initValue;
        z2 = initValue;
    }

    inline bool smoothParameter(T in, T& out)
    {
        if (smootherType == smoothingMethod::kLPFSmoother)
        {
            z = (in * b) + (z * a);
            if (z == z2)
            {
                out = in;
                return false;
            }
            z2 = z;
            out = z2;
            return true;
        }
        else // if (smootherType == smoothingMethod::kLinearSmoother)
        {
            if (in == z)
            {
                out = in;
                return false;
            }
            if (in > z)
            {
                z += linInc;
                if (z > in) z = in;
            }
            else if (in < z)
            {
                z -= linInc;
                if (z < in) z = in;
            }
            out = z;
            return true;
        }
    }

private:
    T a = 0.0;
    T b = 0.0;
    T z = 0.0;
    T z2 = 0.0;

    T linInc = 0.0;

    T minVal = 0.0;
    T maxVal = 1.0;

    T sampleRate = 44100;
    T smoothingTimeInMSec = 100.0;

    smoothingMethod smootherType = smoothingMethod::kLPFSmoother;
};


#endif