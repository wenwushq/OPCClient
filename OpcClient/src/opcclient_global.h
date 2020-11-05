#pragma once


#ifndef BUILD_STATIC
# if defined(OPCCLIENT_LIB)
#  define OPCCLIENT_EXPORT Q_DECL_EXPORT
# else
#  define OPCCLIENT_EXPORT Q_DECL_IMPORT
# endif
#else
# define OPCCLIENT_EXPORT
#endif

// main namespace
#ifndef URUTIL_NAMESPACE
#  define URUTIL_NAMESPACE UrUtil
#  define URUTIL_BEGIN_NAMESPACE namespace URUTIL_NAMESPACE {
#  define URUTIL_END_NAMESPACE }
#  define URUTIL_USE_NAMESPACE using namespace URUTIL_NAMESPACE;
#  define QT_PREPEND_NAMESPACE_URUTIL(name) ::URUTIL_NAMESPACE::name
#endif // !URUTIL_NAMESPACE
