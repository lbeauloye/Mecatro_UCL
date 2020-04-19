/*
 * UAVCAN data structure definition for libuavcan.
 *
 * Autogenerated, do not edit.
 *
 * Source file: /home/westornd/Documents/UCL/Q7/ELME2002/CVRA/C/motor/control/20020.Trajectory.uavcan
 */

#ifndef MOTOR_CONTROL_TRAJECTORY_HPP_INCLUDED
#define MOTOR_CONTROL_TRAJECTORY_HPP_INCLUDED

#include <uavcan/build_config.hpp>
#include <uavcan/node/global_data_type_registry.hpp>
#include <uavcan/marshal/types.hpp>

/******************************* Source text **********************************
#
# Switch to dynamic control to follow trajectory with given current position &
# speed. Position interpolation between updates is done assuming constant speed.
# A feed-forward torque can be applied if given.
#

# UAVCAN node ID for unicast addressing
uint7 node_id

float16 position      # [rad]
float16 velocity      # [rad/s]
float16 acceleration  # [rad/s^2]
float16 torque        # [Nm]
******************************************************************************/

/********************* DSDL signature source definition ***********************
motor.control.Trajectory
saturated uint7 node_id
saturated float16 position
saturated float16 velocity
saturated float16 acceleration
saturated float16 torque
******************************************************************************/

#undef node_id
#undef position
#undef velocity
#undef acceleration
#undef torque

namespace motor
{
namespace control
{

template <int _tmpl>
struct UAVCAN_EXPORT Trajectory_
{
    typedef const Trajectory_<_tmpl>& ParameterType;
    typedef Trajectory_<_tmpl>& ReferenceType;

    struct ConstantTypes
    {
    };

    struct FieldTypes
    {
        typedef ::uavcan::IntegerSpec< 7, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > node_id;
        typedef ::uavcan::FloatSpec< 16, ::uavcan::CastModeSaturate > position;
        typedef ::uavcan::FloatSpec< 16, ::uavcan::CastModeSaturate > velocity;
        typedef ::uavcan::FloatSpec< 16, ::uavcan::CastModeSaturate > acceleration;
        typedef ::uavcan::FloatSpec< 16, ::uavcan::CastModeSaturate > torque;
    };

    enum
    {
        MinBitLen
            = FieldTypes::node_id::MinBitLen
            + FieldTypes::position::MinBitLen
            + FieldTypes::velocity::MinBitLen
            + FieldTypes::acceleration::MinBitLen
            + FieldTypes::torque::MinBitLen
    };

    enum
    {
        MaxBitLen
            = FieldTypes::node_id::MaxBitLen
            + FieldTypes::position::MaxBitLen
            + FieldTypes::velocity::MaxBitLen
            + FieldTypes::acceleration::MaxBitLen
            + FieldTypes::torque::MaxBitLen
    };

    // Constants

    // Fields
    typename ::uavcan::StorageType< typename FieldTypes::node_id >::Type node_id;
    typename ::uavcan::StorageType< typename FieldTypes::position >::Type position;
    typename ::uavcan::StorageType< typename FieldTypes::velocity >::Type velocity;
    typename ::uavcan::StorageType< typename FieldTypes::acceleration >::Type acceleration;
    typename ::uavcan::StorageType< typename FieldTypes::torque >::Type torque;

    Trajectory_()
        : node_id()
        , position()
        , velocity()
        , acceleration()
        , torque()
    {
        ::uavcan::StaticAssert<_tmpl == 0>::check();  // Usage check

#if UAVCAN_DEBUG
        /*
         * Cross-checking MaxBitLen provided by the DSDL compiler.
         * This check shall never be performed in user code because MaxBitLen value
         * actually depends on the nested types, thus it is not invariant.
         */
        ::uavcan::StaticAssert<71 == MaxBitLen>::check();
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
    enum { DefaultDataTypeID = 20020 };

    static const char* getDataTypeFullName()
    {
        return "motor.control.Trajectory";
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
bool Trajectory_<_tmpl>::operator==(ParameterType rhs) const
{
    return
        node_id == rhs.node_id &&
        position == rhs.position &&
        velocity == rhs.velocity &&
        acceleration == rhs.acceleration &&
        torque == rhs.torque;
}

template <int _tmpl>
bool Trajectory_<_tmpl>::isClose(ParameterType rhs) const
{
    return
        ::uavcan::areClose(node_id, rhs.node_id) &&
        ::uavcan::areClose(position, rhs.position) &&
        ::uavcan::areClose(velocity, rhs.velocity) &&
        ::uavcan::areClose(acceleration, rhs.acceleration) &&
        ::uavcan::areClose(torque, rhs.torque);
}

template <int _tmpl>
int Trajectory_<_tmpl>::encode(ParameterType self, ::uavcan::ScalarCodec& codec,
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
    res = FieldTypes::position::encode(self.position, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::velocity::encode(self.velocity, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::acceleration::encode(self.acceleration, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::torque::encode(self.torque, codec,  tao_mode);
    return res;
}

template <int _tmpl>
int Trajectory_<_tmpl>::decode(ReferenceType self, ::uavcan::ScalarCodec& codec,
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
    res = FieldTypes::position::decode(self.position, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::velocity::decode(self.velocity, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::acceleration::decode(self.acceleration, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::torque::decode(self.torque, codec,  tao_mode);
    return res;
}

/*
 * Out of line type method definitions
 */
template <int _tmpl>
::uavcan::DataTypeSignature Trajectory_<_tmpl>::getDataTypeSignature()
{
    ::uavcan::DataTypeSignature signature(0x9728ED04238B933AULL);

    FieldTypes::node_id::extendDataTypeSignature(signature);
    FieldTypes::position::extendDataTypeSignature(signature);
    FieldTypes::velocity::extendDataTypeSignature(signature);
    FieldTypes::acceleration::extendDataTypeSignature(signature);
    FieldTypes::torque::extendDataTypeSignature(signature);

    return signature;
}

/*
 * Out of line constant definitions
 */

/*
 * Final typedef
 */
typedef Trajectory_<0> Trajectory;

namespace
{

const ::uavcan::DefaultDataTypeRegistrator< ::motor::control::Trajectory > _uavcan_gdtr_registrator_Trajectory;

}

} // Namespace control
} // Namespace motor

/*
 * YAML streamer specialization
 */
namespace uavcan
{

template <>
class UAVCAN_EXPORT YamlStreamer< ::motor::control::Trajectory >
{
public:
    template <typename Stream>
    static void stream(Stream& s, ::motor::control::Trajectory::ParameterType obj, const int level);
};

template <typename Stream>
void YamlStreamer< ::motor::control::Trajectory >::stream(Stream& s, ::motor::control::Trajectory::ParameterType obj, const int level)
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
    YamlStreamer< ::motor::control::Trajectory::FieldTypes::node_id >::stream(s, obj.node_id, level + 1);
    s << '\n';
    for (int pos = 0; pos < level; pos++)
    {
        s << "  ";
    }
    s << "position: ";
    YamlStreamer< ::motor::control::Trajectory::FieldTypes::position >::stream(s, obj.position, level + 1);
    s << '\n';
    for (int pos = 0; pos < level; pos++)
    {
        s << "  ";
    }
    s << "velocity: ";
    YamlStreamer< ::motor::control::Trajectory::FieldTypes::velocity >::stream(s, obj.velocity, level + 1);
    s << '\n';
    for (int pos = 0; pos < level; pos++)
    {
        s << "  ";
    }
    s << "acceleration: ";
    YamlStreamer< ::motor::control::Trajectory::FieldTypes::acceleration >::stream(s, obj.acceleration, level + 1);
    s << '\n';
    for (int pos = 0; pos < level; pos++)
    {
        s << "  ";
    }
    s << "torque: ";
    YamlStreamer< ::motor::control::Trajectory::FieldTypes::torque >::stream(s, obj.torque, level + 1);
}

}

namespace motor
{
namespace control
{

template <typename Stream>
inline Stream& operator<<(Stream& s, ::motor::control::Trajectory::ParameterType obj)
{
    ::uavcan::YamlStreamer< ::motor::control::Trajectory >::stream(s, obj, 0);
    return s;
}

} // Namespace control
} // Namespace motor

#endif // MOTOR_CONTROL_TRAJECTORY_HPP_INCLUDED