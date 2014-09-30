require "rautomation"
require File.dirname(__FILE__) + '/common_helpers.rb'
require File.dirname(__FILE__) + '/misc_methods.rb'
require 'thread'

include MiscMethods

module Helpers

  def open_firefox
    @browser_name = "Firefox"
    @password_dialog_title = "Parooli sisestamine on vajalik"
    @browser = Watir::Browser.new :firefox, :profile => "default"
    @autoit = WIN32OLE.new('AutoItX3.Control')
  end

  def open_ie
    @browser_name = "Internet Explorer"
    @password_dialog_title = "Password required"
    @browser = Watir::Browser.new :ie
    @autoit = WIN32OLE.new('AutoItX3.Control')
  end

  def open_chrome
    @browser_name = "Google Chrome"
    @password_dialog_title = "Parooli sisestamine on vajalik"
    @browser = Watir::Browser.new :chrome, :profile => "default"
    @autoit = WIN32OLE.new('AutoItX3.Control')
  end

  def text_area_content id
    @browser.textarea(:id=>id).value
  end

  def hit_escape
    sleep 2
    @autoit.Send("{Escape}")
  end

  def hit_return
    sleep 2
    @autoit.Send("{Enter}")
  end

  def hit_tab
    sleep 2
    @autoit.Send("{TAB}")
  end

  def window_exists title
    @autoit.WinExists(title)!= 0
  end

  def wait_for_window title
    sleep 1
    counter = 0
    while !window_exists(title) do
      counter += 1
      counter.should < 15
      sleep 1
    end
    if window_exists(title) then
      return true
    else
      return false
    end
  end

  def do_fork
    @pid= Thread.new {
      yield
    }
  end

  def unblock_pin1
    system("unblockPIN1.cmd #{@reader_id}")
  end

  def unblock_pin2
    system("unblockPIN2.cmd #{@reader_id}")
  end

  def select_certificate_from_dialog language = 'est', cert_line_number = 1
    cert_selection_window = pick_up_certificate_selection_dialog language, cert_line_number
    puts cert_selection_window.title
    cert_selection_window.button(:value => "Ok").click
    sleep 1
    cert_selection_window
  end

  def show_certificate_from_dialog language = 'est', cert_line_number = 1
    cert_selection_window = pick_up_certificate_selection_dialog language, cert_line_number
    sleep 1

    cert_selection_window
  end

  def pick_up_certificate_selection_dialog language, cert_line_number = 1
    cert_selection_dialog_title = "Windows Security"#load_translation('certificate_selection_dialog_title', language)
    cert_selection_window = RAutomation::Window.new(:title => cert_selection_dialog_title)
    if cert_selection_window.nil?
      raise "Unable to find certificate selection dialog with title #{cert_selection_dialog_title}"
    end

    RAutomation::WaitHelper.wait_until { cert_selection_window.present? }
    cert_line_number.times do
        sleep 1
        cert_selection_window.send_keys(vk_down)
    end
    cert_selection_window
  end

  def activate_cert_selection_window language
    @autoit.WinActivate(load_translation('certificate_selection_dialog_title', language))
  end


  def button_exists_in_dialog? dialog_window, expected_button_label
    button_exists = dialog_window.button(:value => expected_button_label).exists?
    dialog_window.button(:value => expected_button_label).click
    #dialog_window.send_keys(vk_return)
    button_exists
  end

end
