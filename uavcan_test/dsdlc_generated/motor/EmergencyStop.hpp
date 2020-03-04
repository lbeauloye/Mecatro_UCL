/*
 * UAVCAN data structure definition for libuavcan.
 *
 * Autogenerated, do not edit.
 *
 * Source file: /home/westornd/Documents/UCL/Q7/ELME2002/CVRA/C/motor/20000.EmergencyStop.uavcan
 */

#ifndef MOTOR_EMERGENCYSTOP_HPP_INCLUDED
#define MOTOR_EMERGENCYSTOP_HPP_INCLUDED

#include <uavcan/build_config.hpp>
#include <uavcan/node/global_data_type_registry.hpp>
#include <uavcan/marshal/types.hpp>

/******************************* Source text **********************************
#
# Emergency Stop
#
******************************************************************************/

/********************* DSDL signature source definition ***********************
motor.EmergencyStop
******************************************************************************/

namespace motor
{

template <int _tmpl>
struct UAVCAN_EXPORT EmergencyStop_
{
    typedef const EmergencyStop_<_tmpl>& ParameterType;
    typedef EmergencyStop_<_tmpl>& ReferenceType;

    struct ConstantTypes
    {
    };

    struct FieldTypes
    {
    };

    enum
    {
        MinBitLen
    };

    enum
    {
        MaxBitLen
    };

    // Constants

    // Fields

    EmergencyStop_()
    {
        ::uavcan::StaticAssert<_tmpl == 0>::check();  // Usage check

#if UAVCAN_DEBUG
        /*
         * Cross-checking MaxBitLen provided by the DSDL compiler.
         * This check shall never be performed in user code because MaxBitLen value
         * actually depends on the nested types, thus it is not invariant.
         */
        ::uavcan::StaticAssert<0 == MaxBitLen>::check();
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
    enum { DefaultDataTypeID = 20000 };

    static const char* getDataTypeFullName()
    {
        return "motor.EmergencyStop";
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
bool EmergencyStop_<_tmpl>::operator==(ParameterType rhs) const
{
    (void)rhs;
    return true;
}

template <int _tmpl>
bool EmergencyStop_<_tmpl>::isClose(ParameterType rhs) const
{
    (void)rhs;
    return true;
}

template <int _tmpl>
int EmergencyStop_<_tmpl>::encode(ParameterType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    return res;
}

template <int _tmpl>
int EmergencyStop_<_tmpl>::decode(ReferenceType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    return res;
}

/*
 * Out of line type method definitions
 */
template <int _tmpl>
::uavcan::DataTypeSignature EmergencyStop_<_tmpl>::getDataTypeSignature()
{
    ::uavcan::DataTypeSignature signature(0xB8503857707BC770ULL);

    return signature;
}

/*
 * Out of line constant definitions
 */

/*
 * Final typedef
 */
typedef EmergencyStop_<0> EmergencyStop;

namespace
{

const ::uavcan::DefaultDataTypeRegistrator< ::motor::EmergencyStop > _uavcan_gdtr_registrator_EmergencyStop;

}

} // Namespace motor

/*
 * YAML streamer specialization
 */
namespace uavcan
{

template <>
class UAVCAN_EXPORT YamlStreamer< ::motor::EmergencyStop >
{
public:
    template <typename Stream>
    static void stream(Stream& s, ::motor::EmergencyStop::ParameterType obj, const int level);
};

template <typename Stream>
void YamlStreamer< ::motor::EmergencyStop >::stream(Stream& s, ::motor::EmergencyStop::ParameterType obj, const int level)
{
    (void)s;
    (void)obj;
    (void)level;
}

}

namespace motor
{

template <typename Stream>
inline Stream& operator<<(Stream& s, ::motor::EmergencyStop::ParameterType obj)
{
    ::uavcan::YamlStreamer< ::motor::EmergencyStop >::stream(s, obj, 0);
    return s;
}

} // Namespace motor

#endif // MOTOR_EMERGENCYSTOP_HPP_INCLUDED