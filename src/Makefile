# ========================
# Tools
# ========================
CC = cl
LINK = link

CFLAGS = /nologo /Wall /WX /std:c++17 /EHsc /Wv:18
LDFLAGS = /nologo advapi32.lib

# ========================
# Output
# ========================
SVC_EXE = svc.exe
SERVICE_EXE = winkey.exe
PROCESS_EXE = process.exe

# ========================
# Directories
# ========================
SVC_DIR = svc2
SERVICE_DIR = winkey
BUILD_DIR = build

# ========================
# Default target
# ========================
all: dirs $(SVC_EXE) $(SERVICE_EXE) $(PROCESS_EXE)

dirs:
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

# ========================
# svc.exe
# ========================
$(SVC_EXE): \
	$(BUILD_DIR)\svc_main.obj \
	$(BUILD_DIR)\svc_utils.obj \
	$(BUILD_DIR)\svc_install.obj \
	$(BUILD_DIR)\svc_start.obj \
	$(BUILD_DIR)\svc_stop.obj \
	$(BUILD_DIR)\svc_delete.obj
	$(LINK) $(LDFLAGS) $** /out:$(SVC_EXE)

# ========================
# myservice.exe
# ========================
$(SERVICE_EXE): \
	$(BUILD_DIR)\winkey.obj
	$(LINK) $(LDFLAGS) $** /out:$(SERVICE_EXE)

$(PROCESS_EXE): \
	$(BUILD_DIR)\process.obj
	$(LINK) $(LDFLAGS) $** /out:$(PROCESS_EXE)

# ========================
# Compilation rules (EXPLICITES)
# ========================
$(BUILD_DIR)\svc_main.obj:
	$(CC) $(CFLAGS) /c $(SVC_DIR)\main.cpp /Fo:$@

$(BUILD_DIR)\svc_utils.obj:
	$(CC) $(CFLAGS) /c $(SVC_DIR)\utils.cpp /Fo:$@

$(BUILD_DIR)\svc_install.obj:
	$(CC) $(CFLAGS) /c $(SVC_DIR)\install.cpp /Fo:$@

$(BUILD_DIR)\svc_start.obj:
	$(CC) $(CFLAGS) /c $(SVC_DIR)\start.cpp /Fo:$@

$(BUILD_DIR)\svc_stop.obj:
	$(CC) $(CFLAGS) /c $(SVC_DIR)\stop.cpp /Fo:$@

$(BUILD_DIR)\svc_delete.obj:
	$(CC) $(CFLAGS) /c $(SVC_DIR)\delete.cpp /Fo:$@

$(BUILD_DIR)\winkey.obj:
	$(CC) $(CFLAGS) /c $(SERVICE_DIR)\winkey.cpp /Fo:$@

$(BUILD_DIR)\process.obj:
	$(CC) $(CFLAGS) /c $(SERVICE_DIR)\process.cpp /Fo:$@

# ========================
# Clean (SAFE)
# ========================
clean:
	if exist $(BUILD_DIR) del /Q $(BUILD_DIR)\*.obj

fclean: clean
	if exist $(SVC_EXE) del /Q $(SVC_EXE)
	if exist $(SERVICE_EXE) del /Q $(SERVICE_EXE)
	if exist $(PROCESS_EXE) del /Q $(PROCESS_EXE)

re: fclean all

.PHONY: all clean fclean re dirs
