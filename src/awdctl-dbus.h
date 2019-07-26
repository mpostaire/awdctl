/*
 * Generated by gdbus-codegen 2.60.5 from fr.mpostaire.awdctl.xml. DO NOT EDIT.
 *
 * The license of this code is the same as for the D-Bus interface description
 * it was derived from.
 */

#ifndef __AWDCTL_DBUS_H__
#define __AWDCTL_DBUS_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for fr.mpostaire.awdctl.Brightness */

#define TYPE_AWDCTL_BRIGHTNESS (awdctl_brightness_get_type ())
#define AWDCTL_BRIGHTNESS(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_AWDCTL_BRIGHTNESS, AwdctlBrightness))
#define IS_AWDCTL_BRIGHTNESS(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_AWDCTL_BRIGHTNESS))
#define AWDCTL_BRIGHTNESS_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_AWDCTL_BRIGHTNESS, AwdctlBrightnessIface))

struct _AwdctlBrightness;
typedef struct _AwdctlBrightness AwdctlBrightness;
typedef struct _AwdctlBrightnessIface AwdctlBrightnessIface;

struct _AwdctlBrightnessIface
{
  GTypeInterface parent_iface;


  gboolean (*handle_dec_brightness) (
    AwdctlBrightness *object,
    GDBusMethodInvocation *invocation,
    guint arg_value);

  gboolean (*handle_inc_brightness) (
    AwdctlBrightness *object,
    GDBusMethodInvocation *invocation,
    guint arg_value);

  gboolean (*handle_set_brightness) (
    AwdctlBrightness *object,
    GDBusMethodInvocation *invocation,
    guint arg_value);

  guint  (*get_min_percentage) (AwdctlBrightness *object);

  guint  (*get_percentage) (AwdctlBrightness *object);

};

GType awdctl_brightness_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *awdctl_brightness_interface_info (void);
guint awdctl_brightness_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void awdctl_brightness_complete_set_brightness (
    AwdctlBrightness *object,
    GDBusMethodInvocation *invocation);

void awdctl_brightness_complete_inc_brightness (
    AwdctlBrightness *object,
    GDBusMethodInvocation *invocation);

void awdctl_brightness_complete_dec_brightness (
    AwdctlBrightness *object,
    GDBusMethodInvocation *invocation);



/* D-Bus method calls: */
void awdctl_brightness_call_set_brightness (
    AwdctlBrightness *proxy,
    guint arg_value,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean awdctl_brightness_call_set_brightness_finish (
    AwdctlBrightness *proxy,
    GAsyncResult *res,
    GError **error);

gboolean awdctl_brightness_call_set_brightness_sync (
    AwdctlBrightness *proxy,
    guint arg_value,
    GCancellable *cancellable,
    GError **error);

void awdctl_brightness_call_inc_brightness (
    AwdctlBrightness *proxy,
    guint arg_value,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean awdctl_brightness_call_inc_brightness_finish (
    AwdctlBrightness *proxy,
    GAsyncResult *res,
    GError **error);

gboolean awdctl_brightness_call_inc_brightness_sync (
    AwdctlBrightness *proxy,
    guint arg_value,
    GCancellable *cancellable,
    GError **error);

void awdctl_brightness_call_dec_brightness (
    AwdctlBrightness *proxy,
    guint arg_value,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean awdctl_brightness_call_dec_brightness_finish (
    AwdctlBrightness *proxy,
    GAsyncResult *res,
    GError **error);

gboolean awdctl_brightness_call_dec_brightness_sync (
    AwdctlBrightness *proxy,
    guint arg_value,
    GCancellable *cancellable,
    GError **error);



/* D-Bus property accessors: */
guint awdctl_brightness_get_percentage (AwdctlBrightness *object);
void awdctl_brightness_set_percentage (AwdctlBrightness *object, guint value);

guint awdctl_brightness_get_min_percentage (AwdctlBrightness *object);
void awdctl_brightness_set_min_percentage (AwdctlBrightness *object, guint value);


/* ---- */

#define TYPE_AWDCTL_BRIGHTNESS_PROXY (awdctl_brightness_proxy_get_type ())
#define AWDCTL_BRIGHTNESS_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_AWDCTL_BRIGHTNESS_PROXY, AwdctlBrightnessProxy))
#define AWDCTL_BRIGHTNESS_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_AWDCTL_BRIGHTNESS_PROXY, AwdctlBrightnessProxyClass))
#define AWDCTL_BRIGHTNESS_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_AWDCTL_BRIGHTNESS_PROXY, AwdctlBrightnessProxyClass))
#define IS_AWDCTL_BRIGHTNESS_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_AWDCTL_BRIGHTNESS_PROXY))
#define IS_AWDCTL_BRIGHTNESS_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_AWDCTL_BRIGHTNESS_PROXY))

typedef struct _AwdctlBrightnessProxy AwdctlBrightnessProxy;
typedef struct _AwdctlBrightnessProxyClass AwdctlBrightnessProxyClass;
typedef struct _AwdctlBrightnessProxyPrivate AwdctlBrightnessProxyPrivate;

struct _AwdctlBrightnessProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  AwdctlBrightnessProxyPrivate *priv;
};

struct _AwdctlBrightnessProxyClass
{
  GDBusProxyClass parent_class;
};

GType awdctl_brightness_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (AwdctlBrightnessProxy, g_object_unref)
#endif

void awdctl_brightness_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
AwdctlBrightness *awdctl_brightness_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
AwdctlBrightness *awdctl_brightness_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void awdctl_brightness_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
AwdctlBrightness *awdctl_brightness_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
AwdctlBrightness *awdctl_brightness_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_AWDCTL_BRIGHTNESS_SKELETON (awdctl_brightness_skeleton_get_type ())
#define AWDCTL_BRIGHTNESS_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_AWDCTL_BRIGHTNESS_SKELETON, AwdctlBrightnessSkeleton))
#define AWDCTL_BRIGHTNESS_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_AWDCTL_BRIGHTNESS_SKELETON, AwdctlBrightnessSkeletonClass))
#define AWDCTL_BRIGHTNESS_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_AWDCTL_BRIGHTNESS_SKELETON, AwdctlBrightnessSkeletonClass))
#define IS_AWDCTL_BRIGHTNESS_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_AWDCTL_BRIGHTNESS_SKELETON))
#define IS_AWDCTL_BRIGHTNESS_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_AWDCTL_BRIGHTNESS_SKELETON))

typedef struct _AwdctlBrightnessSkeleton AwdctlBrightnessSkeleton;
typedef struct _AwdctlBrightnessSkeletonClass AwdctlBrightnessSkeletonClass;
typedef struct _AwdctlBrightnessSkeletonPrivate AwdctlBrightnessSkeletonPrivate;

struct _AwdctlBrightnessSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  AwdctlBrightnessSkeletonPrivate *priv;
};

struct _AwdctlBrightnessSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType awdctl_brightness_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (AwdctlBrightnessSkeleton, g_object_unref)
#endif

AwdctlBrightness *awdctl_brightness_skeleton_new (void);


/* ------------------------------------------------------------------------ */
/* Declarations for fr.mpostaire.awdctl.Volume */

#define TYPE_AWDCTL_VOLUME (awdctl_volume_get_type ())
#define AWDCTL_VOLUME(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_AWDCTL_VOLUME, AwdctlVolume))
#define IS_AWDCTL_VOLUME(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_AWDCTL_VOLUME))
#define AWDCTL_VOLUME_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), TYPE_AWDCTL_VOLUME, AwdctlVolumeIface))

struct _AwdctlVolume;
typedef struct _AwdctlVolume AwdctlVolume;
typedef struct _AwdctlVolumeIface AwdctlVolumeIface;

struct _AwdctlVolumeIface
{
  GTypeInterface parent_iface;


  gboolean (*handle_dec_volume) (
    AwdctlVolume *object,
    GDBusMethodInvocation *invocation,
    guint arg_value);

  gboolean (*handle_inc_volume) (
    AwdctlVolume *object,
    GDBusMethodInvocation *invocation,
    guint arg_value);

  gboolean (*handle_set_volume) (
    AwdctlVolume *object,
    GDBusMethodInvocation *invocation,
    guint arg_value);

  gboolean (*handle_toggle_volume) (
    AwdctlVolume *object,
    GDBusMethodInvocation *invocation);

  gboolean  (*get_muted) (AwdctlVolume *object);

  guint  (*get_percentage) (AwdctlVolume *object);

};

GType awdctl_volume_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *awdctl_volume_interface_info (void);
guint awdctl_volume_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void awdctl_volume_complete_set_volume (
    AwdctlVolume *object,
    GDBusMethodInvocation *invocation);

void awdctl_volume_complete_inc_volume (
    AwdctlVolume *object,
    GDBusMethodInvocation *invocation);

void awdctl_volume_complete_dec_volume (
    AwdctlVolume *object,
    GDBusMethodInvocation *invocation);

void awdctl_volume_complete_toggle_volume (
    AwdctlVolume *object,
    GDBusMethodInvocation *invocation);



/* D-Bus method calls: */
void awdctl_volume_call_set_volume (
    AwdctlVolume *proxy,
    guint arg_value,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean awdctl_volume_call_set_volume_finish (
    AwdctlVolume *proxy,
    GAsyncResult *res,
    GError **error);

gboolean awdctl_volume_call_set_volume_sync (
    AwdctlVolume *proxy,
    guint arg_value,
    GCancellable *cancellable,
    GError **error);

void awdctl_volume_call_inc_volume (
    AwdctlVolume *proxy,
    guint arg_value,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean awdctl_volume_call_inc_volume_finish (
    AwdctlVolume *proxy,
    GAsyncResult *res,
    GError **error);

gboolean awdctl_volume_call_inc_volume_sync (
    AwdctlVolume *proxy,
    guint arg_value,
    GCancellable *cancellable,
    GError **error);

void awdctl_volume_call_dec_volume (
    AwdctlVolume *proxy,
    guint arg_value,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean awdctl_volume_call_dec_volume_finish (
    AwdctlVolume *proxy,
    GAsyncResult *res,
    GError **error);

gboolean awdctl_volume_call_dec_volume_sync (
    AwdctlVolume *proxy,
    guint arg_value,
    GCancellable *cancellable,
    GError **error);

void awdctl_volume_call_toggle_volume (
    AwdctlVolume *proxy,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean awdctl_volume_call_toggle_volume_finish (
    AwdctlVolume *proxy,
    GAsyncResult *res,
    GError **error);

gboolean awdctl_volume_call_toggle_volume_sync (
    AwdctlVolume *proxy,
    GCancellable *cancellable,
    GError **error);



/* D-Bus property accessors: */
guint awdctl_volume_get_percentage (AwdctlVolume *object);
void awdctl_volume_set_percentage (AwdctlVolume *object, guint value);

gboolean awdctl_volume_get_muted (AwdctlVolume *object);
void awdctl_volume_set_muted (AwdctlVolume *object, gboolean value);


/* ---- */

#define TYPE_AWDCTL_VOLUME_PROXY (awdctl_volume_proxy_get_type ())
#define AWDCTL_VOLUME_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_AWDCTL_VOLUME_PROXY, AwdctlVolumeProxy))
#define AWDCTL_VOLUME_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_AWDCTL_VOLUME_PROXY, AwdctlVolumeProxyClass))
#define AWDCTL_VOLUME_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_AWDCTL_VOLUME_PROXY, AwdctlVolumeProxyClass))
#define IS_AWDCTL_VOLUME_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_AWDCTL_VOLUME_PROXY))
#define IS_AWDCTL_VOLUME_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_AWDCTL_VOLUME_PROXY))

typedef struct _AwdctlVolumeProxy AwdctlVolumeProxy;
typedef struct _AwdctlVolumeProxyClass AwdctlVolumeProxyClass;
typedef struct _AwdctlVolumeProxyPrivate AwdctlVolumeProxyPrivate;

struct _AwdctlVolumeProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  AwdctlVolumeProxyPrivate *priv;
};

struct _AwdctlVolumeProxyClass
{
  GDBusProxyClass parent_class;
};

GType awdctl_volume_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (AwdctlVolumeProxy, g_object_unref)
#endif

void awdctl_volume_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
AwdctlVolume *awdctl_volume_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
AwdctlVolume *awdctl_volume_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void awdctl_volume_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
AwdctlVolume *awdctl_volume_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
AwdctlVolume *awdctl_volume_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define TYPE_AWDCTL_VOLUME_SKELETON (awdctl_volume_skeleton_get_type ())
#define AWDCTL_VOLUME_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_AWDCTL_VOLUME_SKELETON, AwdctlVolumeSkeleton))
#define AWDCTL_VOLUME_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), TYPE_AWDCTL_VOLUME_SKELETON, AwdctlVolumeSkeletonClass))
#define AWDCTL_VOLUME_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_AWDCTL_VOLUME_SKELETON, AwdctlVolumeSkeletonClass))
#define IS_AWDCTL_VOLUME_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_AWDCTL_VOLUME_SKELETON))
#define IS_AWDCTL_VOLUME_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_AWDCTL_VOLUME_SKELETON))

typedef struct _AwdctlVolumeSkeleton AwdctlVolumeSkeleton;
typedef struct _AwdctlVolumeSkeletonClass AwdctlVolumeSkeletonClass;
typedef struct _AwdctlVolumeSkeletonPrivate AwdctlVolumeSkeletonPrivate;

struct _AwdctlVolumeSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  AwdctlVolumeSkeletonPrivate *priv;
};

struct _AwdctlVolumeSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType awdctl_volume_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (AwdctlVolumeSkeleton, g_object_unref)
#endif

AwdctlVolume *awdctl_volume_skeleton_new (void);


G_END_DECLS

#endif /* __AWDCTL_DBUS_H__ */