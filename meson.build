project('sqlitewriter', 'cpp', default_options : ['cpp_std=c++17'])

sqlitedep = dependency('sqlite3', version : '>3')

threaddep = dependency('threads')

jsondep = dependency('nlohmann_json')

#add_executable(demo demo.cc sqlwriter.cc)
#target_link_libraries(demo  sqlite3  pthread  dl  )

sqlitewriter_lib = library(
  'sqlitewriter',
  'sqlwriter.cc',
  'jsonhelper.cc',
  install: false,
  include_directories: '',
  dependencies: [sqlitedep, threaddep, jsondep]
)

sqlitewriter_dep = declare_dependency(
  link_with: sqlitewriter_lib,
  include_directories: '',
)

if meson.version().version_compare('>=0.54.0')
  meson.override_dependency('sqlitewriter', sqlitewriter_dep)
endif


executable('demo', 'demo.cc', 'sqlwriter.cc',  'jsonhelper.cc',
	dependencies: [sqlitedep, threaddep, jsondep])

executable('testrunner', 'testrunner.cc', 'sqlwriter.cc',  'jsonhelper.cc',
	dependencies: [sqlitedep, threaddep, jsondep])

