require 'rubygems'
require "rautomation"

shared_examples_for 'ID-card browser plugin' do
  include Helpers

  before(:all) do
    load_config
    @correct_pin = "01497"
    @wrong_pin = "00000"
    open_browser
  end

  before(:each) do
    @browser.goto @test_page_url
    @browser.select_list(:id, 'pluginLanguage').select_value('est')
  end

  after(:all) do
    @browser.close
  end

  it "TC 2740: should sign hash if valid PIN2 is entered" do
    select_certificate
    do_fork do
      enter_pin @correct_pin
    end
    @browser.link(:text, 'Sign').click
    signature_should_be @signature
  end

  it "TC 2756: should prompt for PIN2 on every sign attempt" do
    select_certificate
    do_fork do
      enter_pin @correct_pin
    end
    @browser.link(:text, 'Sign').click
    signature_should_be @signature

    do_fork do
      enter_pin @correct_pin
    end
    @browser.link(:text, 'Sign').click
    signature_should_be @signature
  end

  it "TC 2744: should sign hash if valid PIN2 is entered on second attempt" do
    select_certificate
    do_fork do
      enter_pin @wrong_pin
      hit_return_if_windows_else_do_nothing
      sleep 4
      enter_pin @correct_pin
    end
    @browser.link(:text, 'Sign').click
    signature_should_be @signature
  end

  it "TC 2746: should cancel signing if dialog cancelled" do
    select_certificate
    do_fork do
      hit_escape
    end
    @browser.link(:text, 'Sign').click
    @browser.div(:id, 'error').text.should =~ /Tehniline viga/ #no proper error management in openxades test page
  end

  it "TC 2748: should block PIN2 after 3 failed attempts" do
    if @browser_name == 'Internet Explorer'
      pending "Test not suitable for #{@browser_name}" do
         raise
      end
    end
    select_certificate
    pin2_blocked_message = nil
    do_fork do
      3.times do
        enter_pin @wrong_pin
      end
      hit_return
      pin2_blocked_message_window = RAutomation::Window.new(:title => "Viga")
      pin2_blocked_message = pin2_blocked_message_window.text
      close_pin2_blocked_message_window
    end
    @browser.link(:text, 'Sign').click

    unblock_pin2
    pin2_blocked_message.should =~ /blokeeritud/i
    @browser.div(:id, 'error').text.should =~ /Tehniline viga/ #no proper error management in openxades test page
  end

 it "TC UNKNOWN: should show dialogs in English" do
    test_certificate_dialog_language 'eng'
  end

  it "TC UNKNOWN: should show dialogs in Estonian" do
    test_certificate_dialog_language 'est'
  end

  it "TC UNKNOWN: should show dialogs in Russian" do
    if is_windows?
      pending "Test not suitable for windows" do
         raise
      end
    end
    test_certificate_dialog_language 'rus'
  end
end

