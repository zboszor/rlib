//------------------------------------------------------------------------------
// <auto-generated />
//
// This file was automatically generated by SWIG (http://www.swig.org).
// Version 3.0.11
//
// Do not make changes to this file unless you know what you are doing--modify
// the SWIG interface file instead.
//------------------------------------------------------------------------------


public class rlib {
  public static SWIGTYPE_p_rlib rlib_init() {
    global::System.IntPtr cPtr = rlibPINVOKE.rlib_init();
    SWIGTYPE_p_rlib ret = (cPtr == global::System.IntPtr.Zero) ? null : new SWIGTYPE_p_rlib(cPtr, false);
    return ret;
  }

  public static int rlib_add_datasource_mysql(SWIGTYPE_p_rlib r, string input_name, string database_host, string database_user, string database_password, string database_database) {
    int ret = rlibPINVOKE.rlib_add_datasource_mysql(SWIGTYPE_p_rlib.getCPtr(r), input_name, database_host, database_user, database_password, database_database);
    return ret;
  }

  public static int rlib_add_datasource_postgres(SWIGTYPE_p_rlib r, string input_name, string conn) {
    int ret = rlibPINVOKE.rlib_add_datasource_postgres(SWIGTYPE_p_rlib.getCPtr(r), input_name, conn);
    return ret;
  }

  public static int rlib_add_datasource_odbc(SWIGTYPE_p_rlib r, string input_name, string source, string user, string password) {
    int ret = rlibPINVOKE.rlib_add_datasource_odbc(SWIGTYPE_p_rlib.getCPtr(r), input_name, source, user, password);
    return ret;
  }

  public static int rlib_add_datasource_xml(SWIGTYPE_p_rlib r, string input_name) {
    int ret = rlibPINVOKE.rlib_add_datasource_xml(SWIGTYPE_p_rlib.getCPtr(r), input_name);
    return ret;
  }

  public static int rlib_add_datasource_csv(SWIGTYPE_p_rlib r, string input_name) {
    int ret = rlibPINVOKE.rlib_add_datasource_csv(SWIGTYPE_p_rlib.getCPtr(r), input_name);
    return ret;
  }

  public static int rlib_add_query_as(SWIGTYPE_p_rlib r, string input_source, string sql, string name) {
    int ret = rlibPINVOKE.rlib_add_query_as(SWIGTYPE_p_rlib.getCPtr(r), input_source, sql, name);
    return ret;
  }

  public static int rlib_add_report(SWIGTYPE_p_rlib r, string name) {
    int ret = rlibPINVOKE.rlib_add_report(SWIGTYPE_p_rlib.getCPtr(r), name);
    return ret;
  }

  public static int rlib_add_report_from_buffer(SWIGTYPE_p_rlib r, string buffer) {
    int ret = rlibPINVOKE.rlib_add_report_from_buffer(SWIGTYPE_p_rlib.getCPtr(r), buffer);
    return ret;
  }

  public static int rlib_execute(SWIGTYPE_p_rlib r) {
    int ret = rlibPINVOKE.rlib_execute(SWIGTYPE_p_rlib.getCPtr(r));
    return ret;
  }

  public static string rlib_get_content_type_as_text(SWIGTYPE_p_rlib r) {
    string ret = rlibPINVOKE.rlib_get_content_type_as_text(SWIGTYPE_p_rlib.getCPtr(r));
    return ret;
  }

  public static int rlib_spool(SWIGTYPE_p_rlib r) {
    int ret = rlibPINVOKE.rlib_spool(SWIGTYPE_p_rlib.getCPtr(r));
    return ret;
  }

  public static int rlib_set_output_format(SWIGTYPE_p_rlib r, int format) {
    int ret = rlibPINVOKE.rlib_set_output_format(SWIGTYPE_p_rlib.getCPtr(r), format);
    return ret;
  }

  public static int rlib_add_resultset_follower_n_to_1(SWIGTYPE_p_rlib r, string leader, string leader_field, string follower, string follower_field) {
    int ret = rlibPINVOKE.rlib_add_resultset_follower_n_to_1(SWIGTYPE_p_rlib.getCPtr(r), leader, leader_field, follower, follower_field);
    return ret;
  }

  public static int rlib_add_resultset_follower(SWIGTYPE_p_rlib r, string leader, string follower) {
    int ret = rlibPINVOKE.rlib_add_resultset_follower(SWIGTYPE_p_rlib.getCPtr(r), leader, follower);
    return ret;
  }

  public static int rlib_set_output_format_from_text(SWIGTYPE_p_rlib r, string name) {
    int ret = rlibPINVOKE.rlib_set_output_format_from_text(SWIGTYPE_p_rlib.getCPtr(r), name);
    return ret;
  }

  public static string rlib_get_output(SWIGTYPE_p_rlib r) {
    string ret = rlibPINVOKE.rlib_get_output(SWIGTYPE_p_rlib.getCPtr(r));
    return ret;
  }

  public static int rlib_get_output_length(SWIGTYPE_p_rlib r) {
    int ret = rlibPINVOKE.rlib_get_output_length(SWIGTYPE_p_rlib.getCPtr(r));
    return ret;
  }

  public static int rlib_signal_connect(SWIGTYPE_p_rlib r, int signal_number, SWIGTYPE_p_f_p_rlib_p_void__int signal_function, SWIGTYPE_p_void data) {
    int ret = rlibPINVOKE.rlib_signal_connect(SWIGTYPE_p_rlib.getCPtr(r), signal_number, SWIGTYPE_p_f_p_rlib_p_void__int.getCPtr(signal_function), SWIGTYPE_p_void.getCPtr(data));
    return ret;
  }

  public static int rlib_signal_connect_string(SWIGTYPE_p_rlib r, string signal_name, SWIGTYPE_p_f_p_rlib_p_void__int signal_function, SWIGTYPE_p_void data) {
    int ret = rlibPINVOKE.rlib_signal_connect_string(SWIGTYPE_p_rlib.getCPtr(r), signal_name, SWIGTYPE_p_f_p_rlib_p_void__int.getCPtr(signal_function), SWIGTYPE_p_void.getCPtr(data));
    return ret;
  }

  public static int rlib_query_refresh(SWIGTYPE_p_rlib r) {
    int ret = rlibPINVOKE.rlib_query_refresh(SWIGTYPE_p_rlib.getCPtr(r));
    return ret;
  }

  public static int rlib_add_parameter(SWIGTYPE_p_rlib r, string name, string value) {
    int ret = rlibPINVOKE.rlib_add_parameter(SWIGTYPE_p_rlib.getCPtr(r), name, value);
    return ret;
  }

  public static int rlib_set_locale(SWIGTYPE_p_rlib r, string locale) {
    int ret = rlibPINVOKE.rlib_set_locale(SWIGTYPE_p_rlib.getCPtr(r), locale);
    return ret;
  }

  public static string rlib_bindtextdomain(SWIGTYPE_p_rlib r, string domainname, string dirname) {
    string ret = rlibPINVOKE.rlib_bindtextdomain(SWIGTYPE_p_rlib.getCPtr(r), domainname, dirname);
    return ret;
  }

  public static void rlib_set_radix_character(SWIGTYPE_p_rlib r, SWIGTYPE_p_gchar radix_character) {
    rlibPINVOKE.rlib_set_radix_character(SWIGTYPE_p_rlib.getCPtr(r), SWIGTYPE_p_gchar.getCPtr(radix_character));
    if (rlibPINVOKE.SWIGPendingException.Pending) throw rlibPINVOKE.SWIGPendingException.Retrieve();
  }

  public static void rlib_set_output_parameter(SWIGTYPE_p_rlib r, string parameter, string value) {
    rlibPINVOKE.rlib_set_output_parameter(SWIGTYPE_p_rlib.getCPtr(r), parameter, value);
  }

  public static void rlib_set_output_encoding(SWIGTYPE_p_rlib r, string encoding) {
    rlibPINVOKE.rlib_set_output_encoding(SWIGTYPE_p_rlib.getCPtr(r), encoding);
  }

  public static int rlib_set_datasource_encoding(SWIGTYPE_p_rlib r, string input_name, string encoding) {
    int ret = rlibPINVOKE.rlib_set_datasource_encoding(SWIGTYPE_p_rlib.getCPtr(r), input_name, encoding);
    return ret;
  }

  public static int rlib_free(SWIGTYPE_p_rlib r) {
    int ret = rlibPINVOKE.rlib_free(SWIGTYPE_p_rlib.getCPtr(r));
    return ret;
  }

  public static string rlib_version() {
    string ret = rlibPINVOKE.rlib_version();
    return ret;
  }

  public static int rlib_graph_add_bg_region(SWIGTYPE_p_rlib r, string graph_name, string region_label, string color, float start, float end) {
    int ret = rlibPINVOKE.rlib_graph_add_bg_region(SWIGTYPE_p_rlib.getCPtr(r), graph_name, region_label, color, start, end);
    return ret;
  }

  public static int rlib_graph_clear_bg_region(SWIGTYPE_p_rlib r, string graph_name) {
    int ret = rlibPINVOKE.rlib_graph_clear_bg_region(SWIGTYPE_p_rlib.getCPtr(r), graph_name);
    return ret;
  }

  public static int rlib_graph_set_x_minor_tick(SWIGTYPE_p_rlib r, string graph_name, string x_value) {
    int ret = rlibPINVOKE.rlib_graph_set_x_minor_tick(SWIGTYPE_p_rlib.getCPtr(r), graph_name, x_value);
    return ret;
  }

  public static int rlib_graph_set_x_minor_tick_by_location(SWIGTYPE_p_rlib r, string graph_name, int location) {
    int ret = rlibPINVOKE.rlib_graph_set_x_minor_tick_by_location(SWIGTYPE_p_rlib.getCPtr(r), graph_name, location);
    return ret;
  }

  public static float rlib_graph(SWIGTYPE_p_rlib r, SWIGTYPE_p_rlib_part part, SWIGTYPE_p_rlib_report report, float left_margin_offset, SWIGTYPE_p_float top_margin_offset) {
    float ret = rlibPINVOKE.rlib_graph(SWIGTYPE_p_rlib.getCPtr(r), SWIGTYPE_p_rlib_part.getCPtr(part), SWIGTYPE_p_rlib_report.getCPtr(report), left_margin_offset, SWIGTYPE_p_float.getCPtr(top_margin_offset));
    return ret;
  }

  public static int rlib_add_search_path(SWIGTYPE_p_rlib r, string path) {
    int ret = rlibPINVOKE.rlib_add_search_path(SWIGTYPE_p_rlib.getCPtr(r), path);
    return ret;
  }

}
