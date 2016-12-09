task :default do
  require 'rubygems'
  require 'copyright_header'

  args = {
    :license_file => './src/license/header',
    :add_path => 'src/influxdb-cpp-rest',
    :output_dir => '.'
  }

  command_line = CopyrightHeader::CommandLine.new( args )
  command_line.execute
end
