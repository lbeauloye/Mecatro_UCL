/*
 * UAVCAN data structure definition for libuavcan.
 *
 * Autogenerated, do not edit.
 *
 * Source file: /home/westornd/Documents/UCL/Q7/ELME2002/CVRA/C/motor/control/20022.Position.uavcan
 */

#ifndef MOTOR_CONTROL_POSITION_HPP_INCLUDED
#define MOTOR_CONTROL_POSITION_HPP_INCLUDED

#include <uavcan/build_config.hpp>
#include <uavcan/node/global_data_type_registry.hpp>
#include <uavcan/marshal/types.hpp>

/******************************* Source text **********************************
#
# Switch to position control and hold given position.
# A trapezoidal velocity profile is automatically generated.
#

# UAVCAN node ID for unicast addressing
uint7 node_id

float32 position        # [rad]
******************************************************************************/

/********************* DSDL signature source definition ***********************
motor.control.Position
saturated uint7 node_id
saturated float32 position
******************************************************************************/

#undef node_id
#undef position

namespace motor
{
namespace control
{

template <int _tmpl>
struct UAVCAN_EXPORT Position_
{
    typedef const Position_<_tmpl>& ParameterType;
    typedef Position_<_tmpl>& ReferenceType;

    struct ConstantTypes
    {
    };

    struct FieldTypes
    {
        typedef ::uavcan::IntegerSpec< 7, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > node_id;
        typedef ::uavcan::FloatSpec< 32, ::uavcan::CastModeSaturate > position;
    };

    enum
    {
        MinBitLen
            = FieldTypes::node_id::MinBitLen
            + FieldTypes::position::MinBitLen
    };

    enum
    {
        MaxBitLen
            = FieldTypes::node_id::MaxBitLen
            + FieldTypes::position::MaxBitLen
    };

    // Constants

    // Fields
    typename ::uavcan::StorageType< typename FieldTypes::node_id >::Type node_id;
    typename ::uavcan::StorageType< typename FieldTypes::position >::Type position;

    Position_()
        : node_id()
        , position()
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
    enum { DefaultDataTypeID = 20022 };

    static const char* getDataTypeFullName()
    {
        return "motor.control.Position";
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
bool Position_<_tmpl>::operator==(ParameterType rhs) const
{
    return
        node_id == rhs.node_id &&
        position == rhs.position;
}

template <int _tmpl>
bool Position_<_tmpl>::isClose(ParameterType rhs) const
{
    return
        ::uavcan::areClose(node_id, rhs.node_id) &&
        ::uavcan::areClose(position, rhs.position);
}

template <int _tmpl>
int Position_<_tmpl>::encode(ParameterType self, ::uavcan::ScalarCodec& codec,
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
    res = FieldTypes::position::encode(self.position, codec,  tao_mode);
    return res;
}

template <int _tmpl>
int Position_<_tmpl>::decode(ReferenceType self, ::uavcan::ScalarCodec& codec,
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
    res = FieldTypes::position::decode(self.position, codec,  tao_mode);
    return res;
}

/*
 * Out of line type method definitions
 */
template <int _tmpl>
::uavcan::DataTypeSignature Position_<_tmpl>::getDataTypeSignature()
{
    ::uavcan::DataTypeSignature signature(0x44251573833B45FDULL);

    FieldTypes::node_id::extendDataTypeSignature(signature);
    FieldTypes::position::extendDataTypeSignature(signature);

    return signature;
}

/*
 * Out of line constant definitions
 */

/*
 * Final typedef
 */
typedef Position_<0> Position;

namespace
{

const ::uavcan::DefaultDataTypeRegistrator< ::motor::control::Position > _uavcan_gdtr_registrator_Position;

}

} // Namespace control
} // Namespace motor

/*
 * YAML streamer specialization
 */
namespace uavcan
{

template <>
class UAVCAN_EXPORT YamlStreamer< ::motor::control::Position >
{
public:
    template <typename Stream>
    static void stream(Stream& s, ::motor::control::Position::ParameterType obj, const int level);
};

template <typename Stream>
void YamlStreamer< ::motor::control::Position >::stream(Stream& s, ::motor::control::Position::ParameterType obj, const int level)
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
    YamlStreamer< ::motor::control::Position::FieldTypes::node_id >::stream(s, obj.node_id, level + 1);
    s << '\n';
    for (int pos = 0; pos < level; pos++)
    {
        s << "  ";
    }
    s << "position: ";
    YamlStreamer< ::motor::control::Position::FieldTypes::position >::stream(s, obj.position, level + 1);
}

}

namespace motor
{
namespace control
{

template <typename Stream>
inline Stream& operator<<(Stream& s, ::motor::control::Position::ParameterType obj)
{
    ::uavcan::YamlStreamer< ::motor::control::Position >::stream(s, obj, 0);
    return s;
}

} // Namespace control
} // Namespace motor

#endif // MOTOR_CONTROL_POSITION_HPP_INCLUDED