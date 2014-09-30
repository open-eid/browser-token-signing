shared_examples_for "Authentication in browser Win" do
  include Helpers

  before(:all) do
    load_config
    @correct_pin = "0090"
    @incorrect_pin = "0000"
    @authentication_url = "https://www.sk.ee/tervitus/index.php"
	# if windows

		if ENV['TMP'].length > 0 then
			@signal_file = File.join(ENV['TMP'],"esteid_authentication_test.signal")
		else
			@signal_file = "/tmp/esteid_authentication_test.signal"
		end
		puts "Signal file: #{@signal_file}"

  end

  before(:each) do
    @pid = nil
    File.delete @signal_file rescue nil
	open_browser	
  end

  after(:each) do
   # Process.kill("KILL", @pid) if @pid rescue nil
    File.delete @signal_file rescue nil
    @browser.close
  end

  def browser_text_should_contain regexp
    @browser.text.should =~ regexp
  end

  def enter_correct_pin
	puts "Oleme siin"
    if !wait_for_window( "Kasutaja identifitseerimisp�ring") then	
			if !wait_for_window( "User Identification Request") then
				wait_for_window( "Choose a digital certificate")
			end				
	end
	puts "Oleme veel "
	hit_return
    wait_for_window @password_dialog_title
    enter_pin @correct_pin
  end

  def verify_unsuccessful_authentication_flow(pin)
    do_fork do
        enter_pin pin
        ret2 = @autoit.WinWait("PIN message", '', 2)
        if (ret2==1)
          @autoit.WinActivate("PIN message")
          hit_return
        end
        @autoit.ControlClick("PIN message",'', 'Retry')
        begin
          wait_for_window "  Windows Security"
          ret = @autoit.WinWait("  Windows Security", '', 2)
          if(ret==1)
            @autoit.WinActivate("  Windows Security")
            @autoit.Send(pin)
            @autoit.ControlClick("  Windows Security",'', 'OK')
          end

          ret2 = @autoit.WinWait("PIN message", '', 2)
          if (ret2==1)
            @autoit.WinActivate("PIN message")
            hit_return
          end
          # @autoit.ControlClick("PIN message",'', 'Retry')
          sleep 1
        end while window_exists "  Windows Security"
    end
    @browser.goto @authentication_url
    @pid.join
    @browser.text.should =~ /Secure Connection Failed|Turvalise .henduse viga/
  end

  def enter_incorrect_and_correct_pin
     enter_pin @incorrect_pin
        ret2 = @autoit.WinWait("PIN message", '', 2)
        if (ret2==1)
          @autoit.WinActivate("PIN message")
          hit_return
        end
        @autoit.ControlClick("PIN message",'', 'Retry')

        wait_for_window "  Windows Security"
          ret = @autoit.WinWait("  Windows Security", '', 2)
          if(ret==1)
            @autoit.WinActivate("  Windows Security")
            @autoit.Send(@correct_pin)
            @autoit.ControlClick("  Windows Security",'', 'OK')
          end
  end

  def verify_normal_authentication_flow
     do_fork do
       enter_pin @correct_pin
     end

    sleep 2
    @browser.goto(@authentication_url)
    @pid.join
    puts @browser.title
    browser_text_should_contain /Tegemist on test kaardiga/
    #irb
    sleep 5
    
  end

 it "TC 2713: should ask for PIN1 and allow access to page" do
   verify_normal_authentication_flow
 end

 it "TC 2719: should not allow access to page if incorrect PIN1 is used multiple times until blocking" do
		begin
			verify_unsuccessful_authentication_flow @incorrect_pin
		ensure
			system "unblockPIN1.cmd #{@reader_id}"
		end
 end

 it "TC 2719: should allow access to page if correct PIN1 is used after incorrect PIN1 while having enough tries left" do
   do_fork do
     enter_incorrect_and_correct_pin
   end
   @browser.goto @authentication_url
   @pid.join
   browser_text_should_contain /Tegemist on test kaardiga/
 end

 it "TC 2721: should authenticate normally when PIN2 is blocked" do
	system "blockPIN.cmd #{@reader_id} 2"
		begin
		  verify_normal_authentication_flow
		ensure
	 	system "unblockPIN2.cmd #{@reader_id}"
		end
 end

  it "TC 2723: should not allow access to page if PIN1 is blocked" do
    system "blockPIN.cmd #{@reader_id} 1"
		begin
			verify_unsuccessful_authentication_flow @correct_pin
		ensure
			system "unblockPIN1.cmd #{@reader_id}"
		end
  end

  it "TC 2729: should show error when authentication is cancelled" do
    pending "Safari doesn't show any errors" if @browser_name == "Safari"
    do_fork do
      if !wait_for_window( "Kasutaja identifitseerimisp�ring") then
			wait_for_window "User Identification Request"
	  end
      begin
        hit_escape
        sleep 2
      end while window_exists("User Identification Request") || window_exists("Kasutaja identifitseerimisp�ring")
    end
    @browser.goto @authentication_url
    @browser.text.should =~ /An error occurred during a connection to www.sk.ee|Esines viga .hendumisel aadressiga www.sk.ee/
	end

  end
