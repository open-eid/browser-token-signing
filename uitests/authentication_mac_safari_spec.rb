#!/usr/bin/spec -cfn

require 'rubygems'
require 'safariwatir'
require File.dirname(__FILE__) + '/mac_helpers.rb'
require File.dirname(__FILE__) + '/authentication_shared.rb'

describe "Authentication in Safari on Mac" do
  it_should_behave_like "Authentication in browser"
	#remove_spec "TC 2729: should show error when authentication is cancelled"

  def open_browser
    open_safari
  end

  def browser_text_should_contain regexp
    sleep 10
    30.times do
      break if @browser.text =~ regexp
      sleep 1
    end
    @browser.text.should =~ regexp
  end

  def choose_identity
    wait_for_window "Choose An Identity"
    while window_exists "Choose An Identity" do
      hit_return
      sleep 2
    end
  end

  def enter_correct_pin
    choose_identity_and_enter_pin @correct_pin
  end

  def choose_identity_and_enter_pin pin
    choose_identity
    wait_for_password_prompt
    enter_text pin
  end

  def verify_unsuccessful_authentication_flow pin
    @browser.goto "http://www.google.ee"
    do_fork do
      7.times { choose_identity_and_enter_pin pin }
      wait_for_window "Choose An Identity"
      hit_escape
    end
    @browser.goto @authentication_url
    wait_for_fork_end    
    @browser.text.should =~ /Google/
  end

  def enter_incorrect_and_correct_pin
    choose_identity_and_enter_pin @incorrect_pin
    choose_identity_and_enter_pin @correct_pin
  end

end