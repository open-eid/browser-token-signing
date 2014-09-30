require File.dirname(__FILE__) + '/common_helpers.rb'

module Helpers

  def open_firefox
    @browser_name = "Firefox"
    @password_dialog_title = ""
    @browser = Watir::Browser.new :firefox, :profile => "default"
    @browser.should_not be nil
    bring_browser_to_front
  end

  def open_safari
    `#{File.dirname(__FILE__)}/../keychain-tool`
    `security lock-keychain "IGOR ŽAIKOVSKI" 2> /dev/null`
    `security lock-keychain "MARI-LIIS MÄNNIK" 2> /dev/null`
    applescript("tell application \"Safari\" to close windows")
    applescript("tell application \"Safari\" to quit")
    sleep 1
    @browser_name = "Safari"
    @browser = Watir::Safari.new
  end

  def applescript script
    `osascript -e '#{script}'`
  end

  def enter_text text
    applescript("tell application \"System Events\" to keystroke \"#{text}\" & return")
  end

  def type_in_browser keystrokes
    bring_browser_to_front
    applescript('delay 0.5')
    applescript("tell application \"System Events\" to #{keystrokes}")
  end

  def bring_browser_to_front
    applescript("tell application \"#{@browser_name}\" to activate")
  end

  def enter_pin pin
    type_in_browser "keystroke \"#{pin}\" & return"
  end

  def hit_escape
    type_in_browser 'key code 53'
  end

  def hit_return
    type_in_browser 'keystroke return'
  end

  def hit_down
    type_in_browser 'key code 125'
  end

  def window_exists title
    applescript("
      global r
      set expectedName to \"#{title}\"
      set r to \"window not found\"
      tell application \"#{@browser_name}\"
          set allNames to (name of every window where visible is true)
          repeat with name in allNames
              if text of name = text of expectedName then
                  set r to \"window found\"
              end
          end
      end
      get r
    ") =~ /window found/
  end

  def wait_for_window title
    applescript "
      set expectedName to \"#{title}\"
      set found to false
      tell application \"#{@browser_name}\"
        repeat until found
          set allNames to (name of every window where visible is true)
          repeat with name in allNames
              if text of name = text of expectedName then
                  set found to true
              end
          end
          delay 0.3
        end
      end
    "
  end

  def wait_for_password_prompt
    until applescript("
        global r
        set r to \"Windows:\"
        tell application \"System Events\"
          set allNames to (name of window of processes)
          repeat with name in allNames
            set r to r & \"\\n\\t[\" & name & \"]\"
          end
        end
        get r
      ") =~ /\[missing value\]/
      sleep 0.5
    end
  end

  def do_fork
    @pid = fork do
      yield
      at_exit { exit! }
    end
  end

  def select_certificate_from_dialog language = 'est', cert_line_number = 1
    applescript("
      tell application \"System Events\"
        tell process \"#{@browser_name}\"
          repeat until sheet 1 of window 1 exists
            delay 0.2
          end repeat
		      repeat #{cert_line_number} times
			      key code 125
		      end repeat
		      keystroke return
        end tell
      end tell
      ")
    sleep 1
  end

  def pick_up_certificate_selection_dialog not_used
    applescript("
      tell application \"System Events\"
        tell process \"#{@browser_name}\"
          repeat until sheet 1 of window 1 exists
            delay 0.2
          end repeat
			    key code 125		      
        end tell
      end tell
      ")
    ""
  end

  def button_exists_in_dialog? not_used, expected_button_label
    #puts ">> button_exists_in_dialog? #{not_used}, #{expected_button_label} for (#{@browser_name})"
    retryCounter = 0
    button_exists = nil
    while button_exists == nil and retryCounter < 10
      button_exists = applescript("
        tell application \"System Events\"
          try
            get title of every button of sheet 1 of window 1 of process \"#{@browser_name}\"
          end try
        end tell
      ") =~ /.*#{expected_button_label}.*/
      retryCounter += 1
      sleep 0.5
    end
    hit_escape
    button_exists != nil ? button_exists : false
  end

end
