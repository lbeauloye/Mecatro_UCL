/*
 * UAVCAN data structure definition for libuavcan.
 *
 * Autogenerated, do not edit.
 *
 * Source file: /home/westornd/Documents/UCL/Q7/ELME2002/CVRA/C/motor/config/203.VelocityPID.uavcan
 */

#ifndef MOTOR_CONFIG_VELOCITYPID_HPP_INCLUDED
#define MOTOR_CONFIG_VELOCITYPID_HPP_INCLUDED

#include <uavcan/build_config.hpp>
#include <uavcan/node/global_data_type_registry.hpp>
#include <uavcan/marshal/types.hpp>

#include <motor/config/PID.hpp>

/******************************* Source text **********************************
PID pid
---
******************************************************************************/

/********************* DSDL signature source definition ***********************
motor.config.VelocityPID
motor.config.PID pid
---
******************************************************************************/

#undef pid

namespace motor
{
namespace config
{

struct UAVCAN_EXPORT VelocityPID_
{
    template <int _tmpl>
    struct Request_
    {
        typedef const Request_<_tmpl>& ParameterType;
        typedef Request_<_tmpl>& ReferenceType;

        struct ConstantTypes
        {
        };

        struct FieldTypes
        {
            typedef ::motor::config::PID pid;
        };

        enum
        {
            MinBitLen
                = FieldTypes::pid::MinBitLen
        };

        enum
        {
            MaxBitLen
                = FieldTypes::pid::MaxBitLen
        };

        // Constants

        // Fields
        typename ::uavcan::StorageType< typename FieldTypes::pid >::Type pid;

        Request_()
            : pid()
        {
            ::uavcan::StaticAssert<_tmpl == 0>::check();  // Usage check

    #if UAVCAN_DEBUG
            /*
             * Cross-checking MaxBitLen provided by the DSDL compiler.
             * This check shall never be performed in user code because MaxBitLen value
             * actually depends on the nested types, thus it is not invariant.
             */
            ::uavcan::StaticAssert<64 == MaxBitLen>::check();
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

    };

    template <int _tmpl>
    struct Response_
    {
        typedef const Response_<_tmpl>& ParameterType;
        typedef Response_<_tmpl>& ReferenceType;

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

        Response_()
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

    };

    typedef Request_<0> Request;
    typedef Response_<0> Response;

    /*
     * Static type info
     */
    enum { DataTypeKind = ::uavcan::DataTypeKindService };
    enum { DefaultDataTypeID = 203 };

    static const char* getDataTypeFullName()
    {
        return "motor.config.VelocityPID";
    }

    static void extendDataTypeSignature(::uavcan::DataTypeSignature& signature)
    {
        signature.extend(getDataTypeSignature());
    }

    static ::uavcan::DataTypeSignature getDataTypeSignature();

private:
    VelocityPID_(); // Don't create objects of this type. Use Request/Response instead.
};

/*
 * Out of line struct method definitions
 */

template <int _tmpl>
bool VelocityPID_::Request_<_tmpl>::operator==(ParameterType rhs) const
{
    return
        pid == rhs.pid;
}

template <int _tmpl>
bool VelocityPID_::Request_<_tmpl>::isClose(ParameterType rhs) const
{
    return
        ::uavcan::areClose(pid, rhs.pid);
}

template <int _tmpl>
int VelocityPID_::Request_<_tmpl>::encode(ParameterType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    res = FieldTypes::pid::encode(self.pid, codec,  tao_mode);
    return res;
}

template <int _tmpl>
int VelocityPID_::Request_<_tmpl>::decode(ReferenceType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    res = FieldTypes::pid::decode(self.pid, codec,  tao_mode);
    return res;
}

template <int _tmpl>
bool VelocityPID_::Response_<_tmpl>::operator==(ParameterType rhs) const
{
    (void)rhs;
    return true;
}

template <int _tmpl>
bool VelocityPID_::Response_<_tmpl>::isClose(ParameterType rhs) const
{
    (void)rhs;
    return true;
}

template <int _tmpl>
int VelocityPID_::Response_<_tmpl>::encode(ParameterType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    return res;
}

template <int _tmpl>
int VelocityPID_::Response_<_tmpl>::decode(ReferenceType self, ::uavcan::ScalarCodec& codec,
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
inline ::uavcan::DataTypeSignature VelocityPID_::getDataTypeSignature()
{
    ::uavcan::DataTypeSignature signature(0xE6B505AF26C59D90ULL);

    Request::FieldTypes::pid::extendDataTypeSignature(signature);

    return signature;
}

/*
 * Out of line constant definitions
 */

/*
 * Final typedef
 */
typedef VelocityPID_ VelocityPID;

namespace
{

const ::uavcan::DefaultDataTypeRegistrator< ::motor::config::VelocityPID > _uavcan_gdtr_registrator_VelocityPID;

}

} // Namespace config
} // Namespace motor

/*
 * YAML streamer specialization
 */
namespace uavcan
{

template <>
class UAVCAN_EXPORT YamlStreamer< ::motor::config::VelocityPID::Request >
{
public:
    template <typename Stream>
    static void stream(Stream& s, ::motor::config::VelocityPID::Request::ParameterType obj, const int level);
};

template <typename Stream>
void YamlStreamer< ::motor::config::VelocityPID::Request >::stream(Stream& s, ::motor::config::VelocityPID::Request::ParameterType obj, const int level)
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
    s << "pid: ";
    YamlStreamer< ::motor::config::VelocityPID::Request::FieldTypes::pid >::stream(s, obj.pid, level + 1);
}

template <>
class UAVCAN_EXPORT YamlStreamer< ::motor::config::VelocityPID::Response >
{
public:
    template <typename Stream>
    static void stream(Stream& s, ::motor::config::VelocityPID::Response::ParameterType obj, const int level);
};

template <typename Stream>
void YamlStreamer< ::motor::config::VelocityPID::Response >::stream(Stream& s, ::motor::config::VelocityPID::Response::ParameterType obj, const int level)
{
    (void)s;
    (void)obj;
    (void)level;
}

}

namespace motor
{
namespace config
{

template <typename Stream>
inline Stream& operator<<(Stream& s, ::motor::config::VelocityPID::Request::ParameterType obj)
{
    ::uavcan::YamlStreamer< ::motor::config::VelocityPID::Request >::stream(s, obj, 0);
    return s;
}

template <typename Stream>
inline Stream& operator<<(Stream& s, ::motor::config::VelocityPID::Response::ParameterType obj)
{
    ::uavcan::YamlStreamer< ::motor::config::VelocityPID::Response >::stream(s, obj, 0);
    return s;
}

} // Namespace config
} // Namespace motor

#endif // MOTOR_CONFIG_VELOCITYPID_HPP_INCLUDED