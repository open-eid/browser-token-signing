def osx_version
  (`sw_vers -productVersion`).scan(/([0-9]+\.[0-9]+)/).flatten.first
end

def mac_download_and_mount_dmg library
  begin
    mount_point = "/Volumes/installer"
    cmd = "ssh administrator@10.0.25.57 'find repository/mac -name \"#{library}-3.*.dmg\"'"
    puts cmd
    library_dmg = (`#{cmd}`).split("\n").first.strip

    dmg_path    = "#{File.basename(library_dmg)}"
    `rm -rf dmg_path`
    `scp administrator@10.0.25.57:#{library_dmg} #{dmg_path}`

    `hdiutil unmount #{mount_point} -force || echo 'not mounted'`
    `hdiutil attach #{dmg_path} -mountpoint #{mount_point}`

    yield mount_point
  ensure
    `hdiutil unmount #{mount_point} -force || echo 'not mounted'`
  end
end

def mac_install_dependencies * libraries
  libraries.each do |library|
    mac_download_and_mount_dmg library do |mount_point|
      `sudo installer -verboseR -pkg #{mount_point}/#{library}.pkg -target /`
    end
  end
end

def get_platform_specific_test_file
  if RUBY_PLATFORM =~ /darwin/
    mac_install_dependencies 'esteidfirefoxplugin'
    return 'plugin_mac_firefox_spec.rb'
  end
  if RUBY_PLATFORM =~ /win/
    return 'windows'
  end
  if RUBY_PLATFORM =~ /linux/
    `sudo apt-get update`
    `sudo apt-get install --yes --force-yes esteidfirefoxplugin`
    return 'plugin_linux_spec.rb'
  end
  return 'unknown'
end


puts `env`
`cp ~/.config/uitests/config.yaml .`
`../unblockPIN2.sh`
`./kill_webdriver_locker.sh` if RUBY_PLATFORM =~ /linux/ or RUBY_PLATFORM =~ /darwin/
`spec -cfn #{get_platform_specific_test_file} -fh:report.html`
exit $?.exitstatus