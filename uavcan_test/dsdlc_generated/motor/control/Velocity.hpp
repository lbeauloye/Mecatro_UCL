/*
 * UAVCAN data structure definition for libuavcan.
 *
 * Autogenerated, do not edit.
 *
 * Source file: /home/westornd/Documents/UCL/Q7/ELME2002/CVRA/C/motor/control/20021.Velocity.uavcan
 */

#ifndef MOTOR_CONTROL_VELOCITY_HPP_INCLUDED
#define MOTOR_CONTROL_VELOCITY_HPP_INCLUDED

#include <uavcan/build_config.hpp>
#include <uavcan/node/global_data_type_registry.hpp>
#include <uavcan/marshal/types.hpp>

/******************************* Source text **********************************
#
# Switch to velocity control and hold given velocity.
#

# UAVCAN node ID for unicast addressing
uint7 node_id

float32 velocity       # [rad/s]
******************************************************************************/

/********************* DSDL signature source definition ***********************
motor.control.Velocity
saturated uint7 node_id
saturated float32 velocity
******************************************************************************/

#undef node_id
#undef velocity

namespace motor
{
namespace control
{

template <int _tmpl>
struct UAVCAN_EXPORT Velocity_
{
    typedef const Velocity_<_tmpl>& ParameterType;
    typedef Velocity_<_tmpl>& ReferenceType;

    struct ConstantTypes
    {
    };

    struct FieldTypes
    {
        typedef ::uavcan::IntegerSpec< 7, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > node_id;
        typedef ::uavcan::FloatSpec< 32, ::uavcan::CastModeSaturate > velocity;
    };

    enum
    {
        MinBitLen
            = FieldTypes::node_id::MinBitLen
            + FieldTypes::velocity::MinBitLen
    };

    enum
    {
        MaxBitLen
            = FieldTypes::node_id::MaxBitLen
            + FieldTypes::velocity::MaxBitLen
    };

    // Constants

    // Fields
    typename ::uavcan::StorageType< typename FieldTypes::node_id >::Type node_id;
    typename ::uavcan::StorageType< typename FieldTypes::velocity >::Type velocity;

    Velocity_()
        : node_id()
        , velocity()
    {
        ::uavcan::StaticAssert<_tmpl == 0>::check();  // Usage check

#if UAVCAN_DEBUG
        /*
         * Cross-checking MaxBitLen provided by the DSDL compiler.
         * This check shall never be performed in user code because MaxBitLen value
         * actually depends on the nested types, thus it is not invariant.
         */
        ::uavcan::StaticAssert<39 == MaxBitLen>::check();
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
    enum { DefaultDataTypeID = 20021 };

    static const char* getDataTypeFullName()
    {
        return "motor.control.Velocity";
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
bool Velocity_<_tmpl>::operator==(ParameterType rhs) const
{
    return
        node_id == rhs.node_id &&
        velocity == rhs.velocity;
}

template <int _tmpl>
bool Velocity_<_tmpl>::isClose(ParameterType rhs) const
{
    return
        ::uavcan::areClose(node_id, rhs.node_id) &&
        ::uavcan::areClose(velocity, rhs.velocity);
}

template <int _tmpl>
int Velocity_<_tmpl>::encode(ParameterType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    res = FieldTypes::node_id::encode(self.node_id, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::velocity::encode(self.velocity, codec,  tao_mode);
    return res;
}

template <int _tmpl>
int Velocity_<_tmpl>::decode(ReferenceType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    res = FieldTypes::node_id::decode(self.node_id, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::velocity::decode(self.velocity, codec,  tao_mode);
    return res;
}

/*
 * Out of line type method definitions
 */
template <int _tmpl>
::uavcan::DataTypeSignature Velocity_<_tmpl>::getDataTypeSignature()
{
    ::uavcan::DataTypeSignature signature(0xAEB1ACFFDA940875ULL);

    FieldTypes::node_id::extendDataTypeSignature(signature);
    FieldTypes::velocity::extendDataTypeSignature(signature);

    return signature;
}

/*
 * Out of line constant definitions
 */

/*
 * Final typedef
 */
typedef Velocity_<0> Velocity;

namespace
{

const ::uavcan::DefaultDataTypeRegistrator< ::motor::control::Velocity > _uavcan_gdtr_registrator_Velocity;

}

} // Namespace control
} // Namespace motor

/*
 * YAML streamer specialization
 */
namespace uavcan
{

template <>
class UAVCAN_EXPORT YamlStreamer< ::motor::control::Velocity >
{
public:
    template <typename Stream>
    static void stream(Stream& s, ::motor::control::Velocity::ParameterType obj, const int level);
};

template <typename Stream>
void YamlStreamer< ::motor::control::Velocity >::stream(Stream& s, ::motor::control::Velocity::ParameterType obj, const int level)
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
    s << "node_id: ";
    YamlStreamer< ::motor::control::Velocity::FieldTypes::node_id >::stream(s, obj.node_id, level + 1);
    s << '\n';
    for (int pos = 0; pos < level; pos++)
    {
        s << "  ";
    }
    s << "velocity: ";
    YamlStreamer< ::motor::control::Velocity::FieldTypes::velocity >::stream(s, obj.velocity, level + 1);
}

}

namespace motor
{
namespace control
{

template <typename Stream>
inline Stream& operator<<(Stream& s, ::motor::control::Velocity::ParameterType obj)
{
    ::uavcan::YamlStreamer< ::motor::control::Velocity >::stream(s, obj, 0);
    return s;
}

} // Namespace control
} // Namespace motor

#endif // MOTOR_CONTROL_VELOCITY_HPP_INCLUDED