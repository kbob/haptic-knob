OPENCM3_DIR := submodules/libopencm3
OPENCM3_LIB := $(OPENCM3_DIR)/lib/libopencm3_stm32f0.a

all: libopencm3


# ---  libopencm3 submodule  -------------------------------------------

     OPENCM3_LIB := $(OPENCM3_DIR)/lib/libopencm3_stm32f0.a

libopencm3: $(OPENCM3_LIB)
        # XXX libopencm3 can't stop rebuilding the world.
        # XXX So if the library exists, do not invoke make.

$(OPENCM3_LIB):
	$(MAKE) -C $(OPENCM3_DIR) TARGETS=stm32/f0


# ---  verify submodules populated  ------------------------------------

ifeq ($(wildcard $(OPENCM3_DIR)/*),)
    missing_submodule := libopencm3
    $(warn hello)
endif

ifdef missing_submodule
    # Hack: newline variable
    # https://stackoverflow.com/questions/17055773
    define n


    endef
    $(error $(missing_submodule) submodule is not initialized.$n\
            please run:$n\
            $$ git submodule init$n\
            $$ git submodule update$n\
            before running make)
endif
