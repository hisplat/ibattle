
$(LOCAL_TARGET)_objects := $($(LOCAL_TARGET)_source:.cpp=.o)

$(LOCAL_CLASS): $(LOCAL_TARGET)
$(LOCAL_TARGET): $($(LOCAL_TARGET)_objects)
	@echo Linking $@ ...
	$(LD) $^ $(LINKS) $(LIBS) -o$@
	@echo $@ done.

