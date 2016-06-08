# This file was automatically generated by SWIG (http://www.swig.org).
# Version 3.0.7
#
# Do not make changes to this file unless you know what you are doing--modify
# the SWIG interface file instead.





from sys import version_info
if version_info >= (2, 6, 0):
    def swig_import_helper():
        from os.path import dirname
        import imp
        fp = None
        try:
            fp, pathname, description = imp.find_module('_rlib', [dirname(__file__)])
        except ImportError:
            import _rlib
            return _rlib
        if fp is not None:
            try:
                _mod = imp.load_module('_rlib', fp, pathname, description)
            finally:
                fp.close()
            return _mod
    _rlib = swig_import_helper()
    del swig_import_helper
else:
    import _rlib
del version_info
try:
    _swig_property = property
except NameError:
    pass  # Python < 2.2 doesn't have 'property'.


def _swig_setattr_nondynamic(self, class_type, name, value, static=1):
    if (name == "thisown"):
        return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'SwigPyObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name, None)
    if method:
        return method(self, value)
    if (not static):
        if _newclass:
            object.__setattr__(self, name, value)
        else:
            self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)


def _swig_setattr(self, class_type, name, value):
    return _swig_setattr_nondynamic(self, class_type, name, value, 0)


def _swig_getattr_nondynamic(self, class_type, name, static=1):
    if (name == "thisown"):
        return self.this.own()
    method = class_type.__swig_getmethods__.get(name, None)
    if method:
        return method(self)
    if (not static):
        return object.__getattr__(self, name)
    else:
        raise AttributeError(name)

def _swig_getattr(self, class_type, name):
    return _swig_getattr_nondynamic(self, class_type, name, 0)


def _swig_repr(self):
    try:
        strthis = "proxy of " + self.this.__repr__()
    except:
        strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

try:
    _object = object
    _newclass = 1
except AttributeError:
    class _object:
        pass
    _newclass = 0



def rlib_init():
    return _rlib.rlib_init()
rlib_init = _rlib.rlib_init

def rlib_add_datasource_mysql(r, input_name, database_host, database_user, database_password, database_database):
    return _rlib.rlib_add_datasource_mysql(r, input_name, database_host, database_user, database_password, database_database)
rlib_add_datasource_mysql = _rlib.rlib_add_datasource_mysql

def rlib_add_datasource_postgres(r, input_name, conn):
    return _rlib.rlib_add_datasource_postgres(r, input_name, conn)
rlib_add_datasource_postgres = _rlib.rlib_add_datasource_postgres

def rlib_add_datasource_odbc(r, input_name, source, user, password):
    return _rlib.rlib_add_datasource_odbc(r, input_name, source, user, password)
rlib_add_datasource_odbc = _rlib.rlib_add_datasource_odbc

def rlib_add_datasource_xml(r, input_name):
    return _rlib.rlib_add_datasource_xml(r, input_name)
rlib_add_datasource_xml = _rlib.rlib_add_datasource_xml

def rlib_add_datasource_csv(r, input_name):
    return _rlib.rlib_add_datasource_csv(r, input_name)
rlib_add_datasource_csv = _rlib.rlib_add_datasource_csv

def rlib_add_query_as(r, input_source, sql, name):
    return _rlib.rlib_add_query_as(r, input_source, sql, name)
rlib_add_query_as = _rlib.rlib_add_query_as

def rlib_add_search_path(r, path):
    return _rlib.rlib_add_search_path(r, path)
rlib_add_search_path = _rlib.rlib_add_search_path

def rlib_add_report(r, name):
    return _rlib.rlib_add_report(r, name)
rlib_add_report = _rlib.rlib_add_report

def rlib_add_report_from_buffer(r, buffer):
    return _rlib.rlib_add_report_from_buffer(r, buffer)
rlib_add_report_from_buffer = _rlib.rlib_add_report_from_buffer

def rlib_execute(r):
    return _rlib.rlib_execute(r)
rlib_execute = _rlib.rlib_execute

def rlib_get_content_type_as_text(r):
    return _rlib.rlib_get_content_type_as_text(r)
rlib_get_content_type_as_text = _rlib.rlib_get_content_type_as_text

def rlib_spool(r):
    return _rlib.rlib_spool(r)
rlib_spool = _rlib.rlib_spool

def rlib_set_output_format(r, format):
    return _rlib.rlib_set_output_format(r, format)
rlib_set_output_format = _rlib.rlib_set_output_format

def rlib_add_resultset_follower_n_to_1(r, leader, leader_field, follower, follower_field):
    return _rlib.rlib_add_resultset_follower_n_to_1(r, leader, leader_field, follower, follower_field)
rlib_add_resultset_follower_n_to_1 = _rlib.rlib_add_resultset_follower_n_to_1

def rlib_add_resultset_follower(r, leader, follower):
    return _rlib.rlib_add_resultset_follower(r, leader, follower)
rlib_add_resultset_follower = _rlib.rlib_add_resultset_follower

def rlib_set_output_format_from_text(r, name):
    return _rlib.rlib_set_output_format_from_text(r, name)
rlib_set_output_format_from_text = _rlib.rlib_set_output_format_from_text

def rlib_get_output(r):
    return _rlib.rlib_get_output(r)
rlib_get_output = _rlib.rlib_get_output

def rlib_get_output_length(r):
    return _rlib.rlib_get_output_length(r)
rlib_get_output_length = _rlib.rlib_get_output_length

def rlib_signal_connect(r, signal_number, signal_function, data):
    return _rlib.rlib_signal_connect(r, signal_number, signal_function, data)
rlib_signal_connect = _rlib.rlib_signal_connect

def rlib_signal_connect_string(r, signal_name, signal_function, data):
    return _rlib.rlib_signal_connect_string(r, signal_name, signal_function, data)
rlib_signal_connect_string = _rlib.rlib_signal_connect_string

def rlib_query_refresh(r):
    return _rlib.rlib_query_refresh(r)
rlib_query_refresh = _rlib.rlib_query_refresh

def rlib_add_parameter(r, name, value):
    return _rlib.rlib_add_parameter(r, name, value)
rlib_add_parameter = _rlib.rlib_add_parameter

def rlib_set_locale(r, locale):
    return _rlib.rlib_set_locale(r, locale)
rlib_set_locale = _rlib.rlib_set_locale

def rlib_bindtextdomain(r, domainname, dirname):
    return _rlib.rlib_bindtextdomain(r, domainname, dirname)
rlib_bindtextdomain = _rlib.rlib_bindtextdomain

def rlib_set_radix_character(r, radix_character):
    return _rlib.rlib_set_radix_character(r, radix_character)
rlib_set_radix_character = _rlib.rlib_set_radix_character

def rlib_set_output_parameter(r, parameter, value):
    return _rlib.rlib_set_output_parameter(r, parameter, value)
rlib_set_output_parameter = _rlib.rlib_set_output_parameter

def rlib_set_output_encoding(r, encoding):
    return _rlib.rlib_set_output_encoding(r, encoding)
rlib_set_output_encoding = _rlib.rlib_set_output_encoding

def rlib_set_datasource_encoding(r, input_name, encoding):
    return _rlib.rlib_set_datasource_encoding(r, input_name, encoding)
rlib_set_datasource_encoding = _rlib.rlib_set_datasource_encoding

def rlib_free(r):
    return _rlib.rlib_free(r)
rlib_free = _rlib.rlib_free

def rlib_version():
    return _rlib.rlib_version()
rlib_version = _rlib.rlib_version

def rlib_graph_add_bg_region(r, graph_name, region_label, color, start, end):
    return _rlib.rlib_graph_add_bg_region(r, graph_name, region_label, color, start, end)
rlib_graph_add_bg_region = _rlib.rlib_graph_add_bg_region

def rlib_graph_clear_bg_region(r, graph_name):
    return _rlib.rlib_graph_clear_bg_region(r, graph_name)
rlib_graph_clear_bg_region = _rlib.rlib_graph_clear_bg_region

def rlib_graph_set_x_minor_tick(r, graph_name, x_value):
    return _rlib.rlib_graph_set_x_minor_tick(r, graph_name, x_value)
rlib_graph_set_x_minor_tick = _rlib.rlib_graph_set_x_minor_tick

def rlib_graph_set_x_minor_tick_by_location(r, graph_name, location):
    return _rlib.rlib_graph_set_x_minor_tick_by_location(r, graph_name, location)
rlib_graph_set_x_minor_tick_by_location = _rlib.rlib_graph_set_x_minor_tick_by_location

def rlib_graph(r, part, report, left_margin_offset, top_margin_offset):
    return _rlib.rlib_graph(r, part, report, left_margin_offset, top_margin_offset)
rlib_graph = _rlib.rlib_graph

def rlib_parse(r):
    return _rlib.rlib_parse(r)
rlib_parse = _rlib.rlib_parse
# This file is compatible with both classic and new-style classes.


