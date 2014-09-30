require File.dirname(__FILE__) + '/common_helpers.rb'

module Helpers

  def open_browser
    @browser_name = "Firefox"
    @password_dialog_title = "Password Required"
    @browser = Watir::Browser.new :firefox, :profile => "default"
  end

  def text_area_content id
    @browser.textarea(:id=>id).value
  end

  def enter_pin pin
    sleep 2
    system("xte 'str #{pin}' 'key Return'")
  end

  def hit_escape
    sleep 2
    system("xte 'key Escape'")
  end

  def hit_return
    sleep 2
    system("xte 'key Return'")
  end

  def window_exists title
    `wmctrl -l | fgrep -c "#{title}"`.chomp != "0"
  end

  def wait_for_window title, time_limit = 15
    sleep 1
    counter = 0
    while !window_exists(title) do
      counter += 1
      counter.should < time_limit
      sleep 1
    end
  end

  def probe_window title, time_limit = 15
    counter = 0
    while !window_exists(title) do
      if counter > time_limit
        return false
      end
      counter += 1
      sleep 1
    end
    true
  end

  def do_fork
    Thread.new {yield}
  end

  def pick_up_certificate_selection_dialog language, cert_line_number = 1
    cert_selection_dialog_title = load_translation('certificate_selection_dialog_title', language)
    wait_for_window cert_selection_dialog_title, 30
    cert_line_number.times do
      system("xte 'key Down'")
    end   
  end

  def select_certificate_from_dialog language = 'est', cert_line_number = 1
    pick_up_certificate_selection_dialog language, cert_line_number
    system("xte 'key Return'")
    sleep 1 # Do not remove this sleep!
  end

  def test_certificate_dialog_language expected_language
    @browser.select_list(:id, 'pluginLanguage').select_value(expected_language)
    window_language = ''
    thread = Thread.new {
      languages = ['est', 'eng', 'rus']
      languages.each do |language|
        cert_selection_dialog_title = load_translation('certificate_selection_dialog_title', language)
        r = probe_window cert_selection_dialog_title, 20
        if (r)
          window_language = language
          break
        end
      end
      hit_escape
    }
    @browser.link(:text, 'Get certificates').click
    thread.join
    window_language.should == expected_language
  end

end
