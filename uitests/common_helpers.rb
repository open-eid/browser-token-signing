require 'yaml'

module Helpers

  def vk_return
    0x0D
  end

  def vk_down
     0x28
  end

  def vk_escape
     0x1B
  end

  def load_config
		begin
			cards = YAML.load_file(File.join(File.dirname(__FILE__), "cards.yaml"))
			config = YAML.load_file(File.join(File.dirname(__FILE__), "config.yaml"))
			card = cards[config["use"]]
			@cardholder = card["name"]
			@signature = /#{card["signature"]}/i
			@reader_id = config["reader_id"]
      @test_page_url = config["test_page_url"]
			puts "Expecting to use card in reader #{@reader_id} with CN \"#{@cardholder}\", card serial #{card["serial"]}"
      puts "Test page URL: #{@test_page_url}"
    rescue => e
			puts "Unable to find cards.yaml and/or config.yaml, please see sample_config.yaml and configure tests!"
      puts "Error: #{e}"
			exit!
		end
  end

  def wait_for_fork_end
    until File.exist?(@signal_file); sleep 0.5; end
  end

  def is_windows?
    RUBY_PLATFORM=~/(win|w)32$/
  end

  def is_linux?
    RUBY_PLATFORM=~/linux$/i
  end

  def block_pin1
    puts "bloking pin1"
    system "./blockPIN.sh #{@reader_id} 1"
  end

  def block_pin2
    puts "bloking pin2"
    system "./blockPIN.sh #{@reader_id} 2"
  end

  def unblock_pin1
    puts "unbloking pin1"
    system("../unblockPIN1.sh 0")
    system("../unblockPIN1.sh 1")
  end

  def unblock_pin2
    puts "unbloking pin2"
    system("../unblockPIN2.sh 0")
    system("../unblockPIN2.sh 1")
  end

  def hit_return_if_windows_else_do_nothing
    # overrided in plugin_windows_ie_spec.rb
  end

  def signature_should_be expected_signature
    20.times do
      break if text_area_content('signatureHex') != ""
      sleep 0.5
    end
    text_area_content('signatureHex').should =~ expected_signature
  end

  def select_certificate
    Thread.new {
      select_certificate_from_dialog
    }
    @browser.link(:text, 'Get certificates').click
    @browser.text_field(:id, 'hashHex').set("26fd6dbee37c55c14af480f00daff8817b61967f")
  end

  def load_translation label, language
    labels = YAML.load_file(File.join(File.dirname(__FILE__), "labels.yaml"))
    labels[label][language]
  end

  def test_certificate_dialog_language expected_language

    if @browser_name == "Safari"
      pending "Test not suitable for #{@browser_name}" do
        raise
      end
    end
    @browser.select_list(:id, 'pluginLanguage').select_value(expected_language)

    correct_cancel_button_exists = nil
    thread = Thread.new {
      cert_selection_window = show_certificate_from_dialog expected_language #pick_up_certificate_selection_dialog expected_language
      expected_cancel_button_label = load_translation('cancel_button_label', expected_language)
      correct_cancel_button_exists = button_exists_in_dialog? cert_selection_window, expected_cancel_button_label
    }
    @browser.link(:text, 'Get certificates').click
    thread.join
    correct_cancel_button_exists.should be_true
  end

end
