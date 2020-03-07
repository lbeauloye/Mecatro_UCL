/*
 * UAVCAN data structure definition for libuavcan.
 *
 * Autogenerated, do not edit.
 *
 * Source file: /home/jyl/Documents/GitHub/MotorBoard/uavcan_test/sirius_cybernetics_corporation/242.GetCurrentTime.uavcan
 */

#ifndef SIRIUS_CYBERNETICS_CORPORATION_GETCURRENTTIME_HPP_INCLUDED
#define SIRIUS_CYBERNETICS_CORPORATION_GETCURRENTTIME_HPP_INCLUDED

#include <uavcan/build_config.hpp>
#include <uavcan/node/global_data_type_registry.hpp>
#include <uavcan/marshal/types.hpp>

#include <uavcan/Timestamp.hpp>

/******************************* Source text **********************************
#
# This service accepts nothing and returns the current time on the server node.
#
# All DSDL definitions should contain a header comment (like this one) that
# explains what this data type is designed for and how to use it.
#

# This service does not accept any parameters, so the request part is empty

---

# Current time.
# Note that the data type "uavcan.Timestamp" is defined by the UAVCAN specification.
uavcan.Timestamp time
******************************************************************************/

/********************* DSDL signature source definition ***********************
sirius_cybernetics_corporation.GetCurrentTime
---
uavcan.Timestamp time
******************************************************************************/

#undef time

namespace sirius_cybernetics_corporation
{

struct UAVCAN_EXPORT GetCurrentTime_
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

        Request_()
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
            typedef ::uavcan::Timestamp time;
        };

        enum
        {
            MinBitLen
                = FieldTypes::time::MinBitLen
        };

        enum
        {
            MaxBitLen
                = FieldTypes::time::MaxBitLen
        };

        // Constants

        // Fields
        typename ::uavcan::StorageType< typename FieldTypes::time >::Type time;

        Response_()
            : time()
        {
            ::uavcan::StaticAssert<_tmpl == 0>::check();  // Usage check

    #if UAVCAN_DEBUG
            /*
             * Cross-checking MaxBitLen provided by the DSDL compiler.
             * This check shall never be performed in user code because MaxBitLen value
             * actually depends on the nested types, thus it is not invariant.
             */
            ::uavcan::StaticAssert<56 == MaxBitLen>::check();
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
    enum { DefaultDataTypeID = 242 };

    static const char* getDataTypeFullName()
    {
        return "sirius_cybernetics_corporation.GetCurrentTime";
    }

    static void extendDataTypeSignature(::uavcan::DataTypeSignature& signature)
    {
        signature.extend(getDataTypeSignature());
    }

    static ::uavcan::DataTypeSignature getDataTypeSignature();

private:
    GetCurrentTime_(); // Don't create objects of this type. Use Request/Response instead.
};

/*
 * Out of line struct method definitions
 */

template <int _tmpl>
bool GetCurrentTime_::Request_<_tmpl>::operator==(ParameterType rhs) const
{
    (void)rhs;
    return true;
}

template <int _tmpl>
bool GetCurrentTime_::Request_<_tmpl>::isClose(ParameterType rhs) const
{
    (void)rhs;
    return true;
}

template <int _tmpl>
int GetCurrentTime_::Request_<_tmpl>::encode(ParameterType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    return res;
}

template <int _tmpl>
int GetCurrentTime_::Request_<_tmpl>::decode(ReferenceType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    return res;
}

template <int _tmpl>
bool GetCurrentTime_::Response_<_tmpl>::operator==(ParameterType rhs) const
{
    return
        time == rhs.time;
}

template <int _tmpl>
bool GetCurrentTime_::Response_<_tmpl>::isClose(ParameterType rhs) const
{
    return
        ::uavcan::areClose(time, rhs.time);
}

template <int _tmpl>
int GetCurrentTime_::Response_<_tmpl>::encode(ParameterType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    res = FieldTypes::time::encode(self.time, codec,  tao_mode);
    return res;
}

template <int _tmpl>
int GetCurrentTime_::Response_<_tmpl>::decode(ReferenceType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    res = FieldTypes::time::decode(self.time, codec,  tao_mode);
    return res;
}

/*
 * Out of line type method definitions
 */
inline ::uavcan::DataTypeSignature GetCurrentTime_::getDataTypeSignature()
{
    ::uavcan::DataTypeSignature signature(0xF5A1FAB56C962564ULL);

    Response::FieldTypes::time::extendDataTypeSignature(signature);

    return signature;
}

/*
 * Out of line constant definitions
 */

/*
 * Final typedef
 */
typedef GetCurrentTime_ GetCurrentTime;

namespace
{

const ::uavcan::DefaultDataTypeRegistrator< ::sirius_cybernetics_corporation::GetCurrentTime > _uavcan_gdtr_registrator_GetCurrentTime;

}

} // Namespace sirius_cybernetics_corporation

/*
 * YAML streamer specialization
 */
namespace uavcan
{

template <>
class UAVCAN_EXPORT YamlStreamer< ::sirius_cybernetics_corporation::GetCurrentTime::Request >
{
public:
    template <typename Stream>
    static void stream(Stream& s, ::sirius_cybernetics_corporation::GetCurrentTime::Request::ParameterType obj, const int level);
};

template <typename Stream>
void YamlStreamer< ::sirius_cybernetics_corporation::GetCurrentTime::Request >::stream(Stream& s, ::sirius_cybernetics_corporation::GetCurrentTime::Request::ParameterType obj, const int level)
{
    (void)s;
    (void)obj;
    (void)level;
}

template <>
class UAVCAN_EXPORT YamlStreamer< ::sirius_cybernetics_corporation::GetCurrentTime::Response >
{
public:
    template <typename Stream>
    static void stream(Stream& s, ::sirius_cybernetics_corporation::GetCurrentTime::Response::ParameterType obj, const int level);
};

template <typename Stream>
void YamlStreamer< ::sirius_cybernetics_corporation::GetCurrentTime::Response >::stream(Stream& s, ::sirius_cybernetics_corporation::GetCurrentTime::Response::ParameterType obj, const int level)
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
    s << "time: ";
    YamlStreamer< ::sirius_cybernetics_corporation::GetCurrentTime::Response::FieldTypes::time >::stream(s, obj.time, level + 1);
}

}

namespace sirius_cybernetics_corporation
{

template <typename Stream>
inline Stream& operator<<(Stream& s, ::sirius_cybernetics_corporation::GetCurrentTime::Request::ParameterType obj)
{
    ::uavcan::YamlStreamer< ::sirius_cybernetics_corporation::GetCurrentTime::Request >::stream(s, obj, 0);
    return s;
}

template <typename Stream>
inline Stream& operator<<(Stream& s, ::sirius_cybernetics_corporation::GetCurrentTime::Response::ParameterType obj)
{
    ::uavcan::YamlStreamer< ::sirius_cybernetics_corporation::GetCurrentTime::Response >::stream(s, obj, 0);
    return s;
}

} // Namespace sirius_cybernetics_corporation

#endif // SIRIUS_CYBERNETICS_CORPORATION_GETCURRENTTIME_HPP_INCLUDED