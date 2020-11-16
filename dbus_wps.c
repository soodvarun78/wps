#include <stdio.h>
#include<glib-2.0/glib.h>
#include<gio/gio.h>
#include<string.h>

#define WPAS_DBUS_NEW_SERVICE "fi.w1.wpa_supplicant1"
#define WPAS_DBUS_NEW_INTRO_INTERFACES "/fi/w1/wpa_supplicant1"
#define WPAS_DBUS_NEW_IFACE_WPS "fi.w1.wpa_supplicant1.Interface.WPS"
#define DBUS_NEW_IFACE "fi.w1.wpa_supplicant1"
#define DBUS_NEW_IFACE_PROP "org.freedesktop.DBus.Properties"
GDBusConnection *conn;
char interface[30];
// Iterates a dictionary of type 'a{sv}'
void
iterate_container_recursive (GVariant *container,char *string)
{
  GVariantIter iter;
  GVariant *child;

  g_variant_iter_init (&iter, container);
  while ((child = g_variant_iter_next_value (&iter)))
    {
	if((strcmp(g_variant_get_type_string (child),"s")==0) && (strcmp(string,"event")==0)){
		printf("%s \n",g_variant_get_string (child,NULL));
	}
	if(strcmp(g_variant_get_type_string (child),"o")==0 && (strcmp(string,"Interface")==0)){
		printf("%s \n",g_variant_get_string (child,NULL));
		strcpy(interface,g_variant_get_string (child,NULL));
	}
	if(strcmp(g_variant_get_type_string (child),"y")==0){
		guint8 x =  g_variant_get_byte(child);
			printf("%c",x);
		//printf("\n");
	}
      if (g_variant_is_container (child))
        iterate_container_recursive (child,string);

      g_variant_unref (child);
    }
}
GVariant *create_wps_builder(){
	GVariantBuilder builder;
	GVariant *vBuilder;
	GVariant *vTuple;
	GVariant *v = g_variant_new_string ("enrollee");

  	g_variant_builder_init (&builder, G_VARIANT_TYPE_ARRAY);

	g_variant_builder_add (&builder, "{sv}", "Role",v);
    	v = g_variant_new_string ("pbc");

	g_variant_builder_add (&builder, "{sv}", "Type",v);

  	vBuilder = g_variant_builder_end (&builder);

	vTuple = g_variant_new_tuple (&vBuilder,1);
	printf("variant type %s \n",g_variant_get_type_string(vTuple));

	return vTuple;

}

void wpa_get_interface(){
        GDBusMessage *rep;
	GError *err = NULL;
	GVariant *x;
        GDBusMessage *msg = g_dbus_message_new_method_call(WPAS_DBUS_NEW_SERVICE,WPAS_DBUS_NEW_INTRO_INTERFACES,DBUS_NEW_IFACE_PROP,"Get");
        g_dbus_message_set_body(msg,g_variant_new("(ss)",DBUS_NEW_IFACE,"Interfaces"));

        if ((rep = g_dbus_connection_send_message_with_reply_sync(conn, msg,
                                        G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &err)) == NULL) {
                printf("rep in NULL \n");
        }
        if (g_dbus_message_get_message_type(rep) == G_DBUS_MESSAGE_TYPE_ERROR) {
                g_dbus_message_to_gerror(rep, &err);
                printf("error is %s \n",err->message);

        }
	x = g_dbus_message_get_body(rep);
	if(x != NULL){
		printf("variant type %s \n",g_variant_get_type_string(x));
		iterate_container_recursive (x,"Interface");
	}
}

void wpa_start_scan(){
        GDBusMessage *rep;
	GError *err=NULL;
        GDBusMessage *msg = g_dbus_message_new_method_call(WPAS_DBUS_NEW_SERVICE,interface,WPAS_DBUS_NEW_IFACE_WPS,"Start");
	GVariant *x = create_wps_builder(); 
        g_dbus_message_set_body(msg,x);

        if ((rep = g_dbus_connection_send_message_with_reply_sync(conn, msg,
                                        G_DBUS_SEND_MESSAGE_FLAGS_NONE, -1, NULL, NULL, &err)) == NULL) {
                printf("rep in NULL \n");
        }
        printf("return msg type  %d \n",g_dbus_message_get_message_type(rep));
        if (g_dbus_message_get_message_type(rep) == G_DBUS_MESSAGE_TYPE_ERROR) {
                g_dbus_message_to_gerror(rep, &err);
                printf("error is %s \n",err->message);

        }
	x = g_dbus_message_get_body(rep);
	if(x != NULL){
		printf("variant type %s \n",g_variant_get_type_string(x));
	}
}

void signal_handler_wps(GDBusConnection *conn,const gchar *sender_name,const gchar *object_path,const gchar *interface_name,const gchar *signal_name,GVariant *parameters,gpointer user_data){
	printf("signal name %s \n",signal_name);
	printf("interface %s \n",interface_name);
	printf("variant type %s \n",g_variant_get_type_string(parameters));
	iterate_container_recursive (parameters,"event");
}
int main(){
	gchar* address;
	GMainLoop *loop = NULL;
	GError *err = NULL;	
	address = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);

	 printf("address %s \n",address);
         conn =  g_dbus_connection_new_for_address_sync(address,
                                        G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
                                        G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION,
                                        NULL, NULL, &err); {
                printf("conn %p \n",conn);
                printf("Couldn't obtain D-Bus connection: %s\n",err?err->message:"NULL");
        //        return EXIT_FAILURE;
        }

	wpa_get_interface();
	wpa_start_scan();
	
	g_dbus_connection_signal_subscribe (conn,WPAS_DBUS_NEW_SERVICE,/*"fi.w1.wpa_supplicant1.BSS"*/"fi.w1.wpa_supplicant1.Interface.WPS",NULL,NULL,NULL,G_DBUS_SIGNAL_FLAGS_NONE,signal_handler_wps,NULL,NULL);
	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

}
