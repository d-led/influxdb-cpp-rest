task :default do
  require 'rubygems'
  require 'copyright_header'

  args = {
    :license_file => './src/license/header',
    :add_path => 'src/influxdb-cpp-rest',
    :add_path => 'src/demo',
    :add_path => 'src/test',
    :add_path => 'src/auth_test',
    :output_dir => '.'
  }

  command_line = CopyrightHeader::CommandLine.new( args )
  command_line.execute
end
