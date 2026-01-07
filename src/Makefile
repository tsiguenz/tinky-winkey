SVC = .\svc.exe
SVC_DIR = svc
TINKY_DIR = tinky
WINKEY_DIR = winkey
MAKE = nmake
MAKE_FLAG = /NOLOGO
WINKEY_LOG_FILE =  C:\Windows\Temp\winkey.log

all:
	cd $(SVC_DIR) && $(MAKE) $(MAKE_FLAG)
	cd $(TINKY_DIR) && $(MAKE) $(MAKE_FLAG)
	cd $(WINKEY_DIR) && $(MAKE) $(MAKE_FLAG)
	if exist $(SVC) $(SVC) install
	if exist $(SVC) $(SVC) start

clean:
	if exist $(SVC) $(SVC) delete 
	del /Q /F $(WINKEY_LOG_FILE)
	cd $(SVC_DIR) && $(MAKE) $(MAKE_FLAG) clean
	cd $(TINKY_DIR) && $(MAKE) $(MAKE_FLAG) clean
	cd $(WINKEY_DIR) && $(MAKE) $(MAKE_FLAG) clean

fclean: clean
	cd $(SVC_DIR) && $(MAKE) $(MAKE_FLAG) fclean
	cd $(TINKY_DIR) && $(MAKE) $(MAKE_FLAG) fclean
	cd $(WINKEY_DIR) && $(MAKE) $(MAKE_FLAG) fclean
	
re:
	cd $(SVC_DIR) && $(MAKE) $(MAKE_FLAG) re
	cd $(TINKY_DIR) && $(MAKE) $(MAKE_FLAG) re
	cd $(WINKEY_DIR) && $(MAKE) $(MAKE_FLAG) re

log:
	type $(WINKEY_LOG_FILE)

.PHONY: all clean fclean re