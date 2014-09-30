
C:
cd \work\browser_tests\uitests
    @echo off    
    set cur_yyyy=%date:~8,4%
    set cur_mm=%date:~5,2%
    set cur_dd=%date:~2,2%

    set cur_hh=%time:~0,2%
    if %cur_hh% lss 10 (set cur_hh=0%time:~1,1%)
    set cur_nn=%time:~3,2%
    set cur_ss=%time:~6,2%
    set cur_ms=%time:~9,2%

    rem Set the timestamp format
    set timestamp=%cur_yyyy%_%cur_mm%_%cur_dd%-%cur_hh%.%cur_nn%.%cur_ss%.%cur_ms%
@echo on
spec  -fh:%USERPROFILE%\Desktop\GC_report.html -fh:C:\work\browser_tests\test_reports\current\GC_report.html -fh:C:\work\browser_tests\test_reports\%timestamp%\GC_report.html -cfn .\*_windows_chrome_spec.rb

@echo off
    rem Clear the environment variables
    set cur_yyyy=
    set cur_mm=
    set cur_dd=
    set cur_hh=
    set cur_nn=
    set cur_ss=
    set cur_ms=
    set timestamp=
	@echo on

pause