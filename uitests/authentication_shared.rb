shared_examples_for "Authentication in browser" do
  include Helpers

  before(:all) do
    load_config
    @correct_pin = "0090"
    @incorrect_pin = "0000"
    @authentication_url = "https://www.sk.ee/tervitus/index.php"
    @signal_file = "/tmp/esteid_authentication_test.signal"
  end

  before(:each) do
    @pid = nil
    File.delete @signal_file rescue nil
    open_browser
  end

  after(:each) do
    Process.kill("KILL", @pid) if @pid rescue nil
    File.delete @signal_file rescue nil
    @browser.close
  end

  def browser_text_should_contain regexp
    @browser.text.should =~ regexp
  end

  def enter_correct_pin
    wait_for_window "User Identification Request"
    hit_return
    wait_for_window @password_dialog_title
    enter_pin @correct_pin
  end

  def verify_unsuccessful_authentication_flow(pin)
    do_fork do
      wait_for_window "User Identification Request"
      hit_return
      begin
        wait_for_window @password_dialog_title
        enter_pin pin
        sleep 2
      end while window_exists @password_dialog_title
    end
    @browser.goto @authentication_url
    @browser.text.should =~ /Secure Connection Failed/
    unblock_pin1
  end

  def enter_incorrect_and_correct_pin
    wait_for_window "User Identification Request"
    hit_return
    wait_for_window @password_dialog_title
    enter_pin @incorrect_pin
    wait_for_window @password_dialog_title
    enter_pin @correct_pin
  end

  def verify_normal_authentication_flow
    do_fork do
      enter_correct_pin
    end
    @browser.goto @authentication_url
    browser_text_should_contain /Tegemist on test kaardiga/
  end

  it "TC 2713: should ask for PIN1 and allow access to page" do
    verify_normal_authentication_flow
  end

  it "TC 2719: should not allow access to page if incorrect PIN1 is used multiple times until blocking" do
		begin
			verify_unsuccessful_authentication_flow @incorrect_pin
		ensure
			system "./unblockPIN1.sh #{@reader_id}"
		end
  end

  it "TC 2719: should allow access to page if correct PIN1 is used after incorrect PIN1 while having enough tries left" do
    do_fork do
      enter_incorrect_and_correct_pin
    end
    @browser.goto @authentication_url
    browser_text_should_contain /Tegemist on test kaardiga/
  end

  it "TC 2721: should authenticate normally when PIN2 is blocked" do
    block_pin2
	begin
      verify_normal_authentication_flow
	ensure
	  unblock_pin2
	end
  end

  it "TC 2723: should not allow access to page if PIN1 is blocked" do
    block_pin1
	begin
		verify_unsuccessful_authentication_flow @correct_pin
    ensure
        unblock_pin1
	end
  end

  it "TC 2729: should show error when authentication is cancelled" do
    pending "Safari doesn't show any errors" if @browser_name == "Safari"
    do_fork do
      wait_for_window "User Identification Request"
      begin
        hit_escape
        sleep 2
      end while window_exists "User Identification Request"
    end
    @browser.goto @authentication_url
    @browser.text.should =~ /An error occurred during a connection to www.sk.ee/
  end
end
