/*
 * UAVCAN data structure definition for libuavcan.
 *
 * Autogenerated, do not edit.
 *
 * Source file: /home/westornd/Documents/UCL/Q7/ELME2002/CVRA/C/motor/feedback/20040.CurrentPID.uavcan
 */

#ifndef MOTOR_FEEDBACK_CURRENTPID_HPP_INCLUDED
#define MOTOR_FEEDBACK_CURRENTPID_HPP_INCLUDED

#include <uavcan/build_config.hpp>
#include <uavcan/node/global_data_type_registry.hpp>
#include <uavcan/marshal/types.hpp>

/******************************* Source text **********************************
#
# Stream to tune the current PID
#

float16 current_setpoint
float16 current
float16 motor_voltage
******************************************************************************/

/********************* DSDL signature source definition ***********************
motor.feedback.CurrentPID
saturated float16 current_setpoint
saturated float16 current
saturated float16 motor_voltage
******************************************************************************/

#undef current_setpoint
#undef current
#undef motor_voltage

namespace motor
{
namespace feedback
{

template <int _tmpl>
struct UAVCAN_EXPORT CurrentPID_
{
    typedef const CurrentPID_<_tmpl>& ParameterType;
    typedef CurrentPID_<_tmpl>& ReferenceType;

    struct ConstantTypes
    {
    };

    struct FieldTypes
    {
        typedef ::uavcan::FloatSpec< 16, ::uavcan::CastModeSaturate > current_setpoint;
        typedef ::uavcan::FloatSpec< 16, ::uavcan::CastModeSaturate > current;
        typedef ::uavcan::FloatSpec< 16, ::uavcan::CastModeSaturate > motor_voltage;
    };

    enum
    {
        MinBitLen
            = FieldTypes::current_setpoint::MinBitLen
            + FieldTypes::current::MinBitLen
            + FieldTypes::motor_voltage::MinBitLen
    };

    enum
    {
        MaxBitLen
            = FieldTypes::current_setpoint::MaxBitLen
            + FieldTypes::current::MaxBitLen
            + FieldTypes::motor_voltage::MaxBitLen
    };

    // Constants

    // Fields
    typename ::uavcan::StorageType< typename FieldTypes::current_setpoint >::Type current_setpoint;
    typename ::uavcan::StorageType< typename FieldTypes::current >::Type current;
    typename ::uavcan::StorageType< typename FieldTypes::motor_voltage >::Type motor_voltage;

    CurrentPID_()
        : current_setpoint()
        , current()
        , motor_voltage()
    {
        ::uavcan::StaticAssert<_tmpl == 0>::check();  // Usage check

#if UAVCAN_DEBUG
        /*
         * Cross-checking MaxBitLen provided by the DSDL compiler.
         * This check shall never be performed in user code because MaxBitLen value
         * actually depends on the nested types, thus it is not invariant.
         */
        ::uavcan::StaticAssert<48 == MaxBitLen>::check();
#endif
    }

    bool operator==(ParameterType rhs) const;
    bool operator!=(ParameterType rhs) const { return !operator==(rhs); }

    /**
     * This comparison is based on @ref uavcan::areClose(), which ensures proper comparison of
     * floating point fields at any depth.
     */
    bool isClose(ParameterType rhs) const;

    static int encode(ParameterType self, ::uavcan::ScalarCodec& codec,
                      ::uavcan::TailArrayOptimizationMode tao_mode = ::uavcan::TailArrayOptEnabled);

    static int decode(ReferenceType self, ::uavcan::ScalarCodec& codec,
                      ::uavcan::TailArrayOptimizationMode tao_mode = ::uavcan::TailArrayOptEnabled);

    /*
     * Static type info
     */
    enum { DataTypeKind = ::uavcan::DataTypeKindMessage };
    enum { DefaultDataTypeID = 20040 };

    static const char* getDataTypeFullName()
    {
        return "motor.feedback.CurrentPID";
    }

    static void extendDataTypeSignature(::uavcan::DataTypeSignature& signature)
    {
        signature.extend(getDataTypeSignature());
    }

    static ::uavcan::DataTypeSignature getDataTypeSignature();

};

/*
 * Out of line struct method definitions
 */

template <int _tmpl>
bool CurrentPID_<_tmpl>::operator==(ParameterType rhs) const
{
    return
        current_setpoint == rhs.current_setpoint &&
        current == rhs.current &&
        motor_voltage == rhs.motor_voltage;
}

template <int _tmpl>
bool CurrentPID_<_tmpl>::isClose(ParameterType rhs) const
{
    return
        ::uavcan::areClose(current_setpoint, rhs.current_setpoint) &&
        ::uavcan::areClose(current, rhs.current) &&
        ::uavcan::areClose(motor_voltage, rhs.motor_voltage);
}

template <int _tmpl>
int CurrentPID_<_tmpl>::encode(ParameterType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    res = FieldTypes::current_setpoint::encode(self.current_setpoint, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::current::encode(self.current, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::motor_voltage::encode(self.motor_voltage, codec,  tao_mode);
    return res;
}

template <int _tmpl>
int CurrentPID_<_tmpl>::decode(ReferenceType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    res = FieldTypes::current_setpoint::decode(self.current_setpoint, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::current::decode(self.current, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::motor_voltage::decode(self.motor_voltage, codec,  tao_mode);
    return res;
}

/*
 * Out of line type method definitions
 */
template <int _tmpl>
::uavcan::DataTypeSignature CurrentPID_<_tmpl>::getDataTypeSignature()
{
    ::uavcan::DataTypeSignature signature(0xE80BDF25C8CE6F3ULL);

    FieldTypes::current_setpoint::extendDataTypeSignature(signature);
    FieldTypes::current::extendDataTypeSignature(signature);
    FieldTypes::motor_voltage::extendDataTypeSignature(signature);

    return signature;
}

/*
 * Out of line constant definitions
 */

/*
 * Final typedef
 */
typedef CurrentPID_<0> CurrentPID;

namespace
{

const ::uavcan::DefaultDataTypeRegistrator< ::motor::feedback::CurrentPID > _uavcan_gdtr_registrator_CurrentPID;

}

} // Namespace feedback
} // Namespace motor

/*
 * YAML streamer specialization
 */
namespace uavcan
{

template <>
class UAVCAN_EXPORT YamlStreamer< ::motor::feedback::CurrentPID >
{
public:
    template <typename Stream>
    static void stream(Stream& s, ::motor::feedback::CurrentPID::ParameterType obj, const int level);
};

template <typename Stream>
void YamlStreamer< ::motor::feedback::CurrentPID >::stream(Stream& s, ::motor::feedback::CurrentPID::ParameterType obj, const int level)
{
    (void)s;
    (void)obj;
    (void)level;
    if (level > 0)
    {
        s << '\n';
        for (int pos = 0; pos < level; pos++)
        {
            s << "  ";
        }
    }
    s << "current_setpoint: ";
    YamlStreamer< ::motor::feedback::CurrentPID::FieldTypes::current_setpoint >::stream(s, obj.current_setpoint, level + 1);
    s << '\n';
    for (int pos = 0; pos < level; pos++)
    {
        s << "  ";
    }
    s << "current: ";
    YamlStreamer< ::motor::feedback::CurrentPID::FieldTypes::current >::stream(s, obj.current, level + 1);
    s << '\n';
    for (int pos = 0; pos < level; pos++)
    {
        s << "  ";
    }
    s << "motor_voltage: ";
    YamlStreamer< ::motor::feedback::CurrentPID::FieldTypes::motor_voltage >::stream(s, obj.motor_voltage, level + 1);
}

}

namespace motor
{
namespace feedback
{

template <typename Stream>
inline Stream& operator<<(Stream& s, ::motor::feedback::CurrentPID::ParameterType obj)
{
    ::uavcan::YamlStreamer< ::motor::feedback::CurrentPID >::stream(s, obj, 0);
    return s;
}

} // Namespace feedback
} // Namespace motor

#endif // MOTOR_FEEDBACK_CURRENTPID_HPP_INCLUDED