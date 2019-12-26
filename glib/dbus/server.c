#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>

#define DBUS_SERVICE "com.deepin.daemon.ACL"
#define DBUS_NET_PATH "/com/deepin/daemon/ACL/Network"
#define DBUS_NET_IFC "com.deepin.daemon.ACL.Network"

static void on_acquired_cb(GDBusConnection *conn, const gchar *name, gpointer user_data);
static void on_name_acquired_cb(GDBusConnection *conn, const gchar *name, gpointer user_data);
static void on_name_lost_cb(GDBusConnection *conn, const gchar *name, gpointer user_data);
static void acl_network_register(GDBusConnection *conn, GError **error);
static void acl_network_handle_method_call(GDBusConnection *conn,
					   const gchar *sender, const gchar *obj_path,
					   const gchar *ifc_name, const gchar *method_name,
					   GVariant *params, GDBusMethodInvocation *invocation, gpointer user_data);
static void emit_network_signal(GDBusConnection *conn, const gchar *sig_name, GVariant *params);
static void handle_network_enable(GVariant *params, GDBusMethodInvocation *invocation);

static GMainLoop *loop = NULL;
static GDBusNodeInfo *net_node_info = NULL;

static const char *server_xml =
	"<node>"
	"    <interface name='com.deepin.daemon.ACL.Network'>"
	"        <method name='Enable'>"
	"            <arg type='s' name='app' direction='in' />"
	"            <arg type='b' name='enabled' direction='in' />"
	"        </method>"
	"        <signal name='AccessRequest'>"
	"            <arg type='s' name='app' direction='out' />"
	"        </signal>"
	"    </interface>"
	"</node>";
static const GDBusInterfaceVTable net_vtable = {
	.method_call = acl_network_handle_method_call,
	.get_property = NULL,
	.set_property = NULL,
};

int
main()
{
	int ret = 0;
	guint owner_id = 0;
	GError *error = NULL;

	net_node_info = g_dbus_node_info_new_for_xml(server_xml, &error);
	if (error) {
		g_error("failed to build network ifc node info: %s\n", error->message);
		return -1;
	}

	owner_id = g_bus_own_name(G_BUS_TYPE_SESSION, DBUS_SERVICE,
				G_BUS_NAME_OWNER_FLAGS_DO_NOT_QUEUE,
				on_acquired_cb,
				on_name_acquired_cb,
				on_name_lost_cb,
				NULL, NULL);
	if (!owner_id) {
		g_error("failed to own bus name: %s\n", DBUS_SERVICE);
		return -1;
	}

	loop = g_main_loop_new(NULL, FALSE);
	if (!loop) {
		g_error("failed to create loop\n");
		ret = -1;
		goto out;
	}
	g_main_loop_run(loop);

out:
	g_dbus_node_info_unref(net_node_info);
	g_bus_unown_name(owner_id);
	return ret;
}

static void
on_acquired_cb(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
	GError *error = NULL;

	g_print("Acquired, start to register network interface\n");
	acl_network_register(conn, &error);
	if (error) {
		g_warning("failed to register network interface: %s\n", error->message);
		g_error_free(error);
		error = NULL;
	}
}

static void
on_name_acquired_cb(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
	g_print("Acquired name: %s\n", name);
}

static void
on_name_lost_cb(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
	g_print("Lost name: %s\n", name);
	g_main_loop_quit(loop);
}

static void
acl_network_register(GDBusConnection *conn, GError **error)
{
	guint register_id = 0;

	register_id = g_dbus_connection_register_object(conn, DBUS_NET_PATH,
							net_node_info->interfaces[0],
							&net_vtable, NULL, NULL, error);
	if (register_id == 0)
		g_warning("failed to register network interface\n");
}

static void
acl_network_handle_method_call(GDBusConnection *conn,
			       const gchar *sender, const gchar *obj_path,
			       const gchar *ifc_name, const gchar *method_name,
			       GVariant *params, GDBusMethodInvocation *invocation, gpointer user_data)
{
	if (strcmp(method_name, "Enable") == 0) {
		handle_network_enable(params, invocation);
	} else {
		g_warning("invalid network method call: %s\n", method_name);
		return;
	}
}

static void
emit_network_signal(GDBusConnection *conn, const gchar *sig_name, GVariant *params)
{
	GError *error = NULL;

	g_dbus_connection_emit_signal(conn, DBUS_SERVICE, DBUS_NET_PATH, DBUS_NET_IFC,
				      sig_name, params, &error);
	if (error) {
		g_warning("failed to emit network signal(%s): %s\n", sig_name, error->message);
		g_error_free(error);
		error = NULL;
	}
}

static void
handle_network_enable(GVariant *params, GDBusMethodInvocation *invocation)
{
	gchar *app = NULL;
	gboolean enabled = FALSE;

	g_variant_get(params, "(sb)", &app, &enabled);
	g_print("Network method Enable call --> app: %s, enabled: %d\n", app, enabled);
	emit_network_signal(g_dbus_method_invocation_get_connection(invocation),
			    "AccessRequest", g_variant_new("(s)", app));
	g_dbus_method_invocation_return_value(invocation, g_variant_new("()"));
}
