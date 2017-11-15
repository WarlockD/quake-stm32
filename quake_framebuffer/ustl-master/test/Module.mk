################ Source files ##########################################

test/SRCS	:= $(wildcard test/?????.cc)
test/TESTS	:= $(addprefix $O,$(test/SRCS:.cc=))
test/OBJS	:= $(addprefix $O,$(test/SRCS:.cc=.o))
test/RPATH	:= $(abspath $O.)
ifdef BUILD_STATIC
test/LIBS	:= ${LIBA}
else
test/LIBS	:= -L${test/RPATH} -l${NAME}
endif
ifdef BUILD_SHARED
test/LIBS	:= -Wl,-rpath,${test/RPATH} ${test/LIBS}
endif
ifdef NOLIBSTDCPP
test/LIBS	+= ${STAL_LIBS} -lm
endif
test/DEPS	:= ${test/OBJS:.o=.d} $Otest/stdtest.d
test/OUTS	:= $(addprefix $O,$(test/SRCS:.cc=.out))

################ Compilation ###########################################

.PHONY:	test/all test/run test/clean test/check

test/all:	${test/TESTS}

# The correct output of a test is stored in testXX.std
# When the test runs, its output is compared to .std
#
test/run:	${test/TESTS}
	@echo "Running build verification tests:";\
	export DYLD_LIBRARY_PATH="${test/RPATH}";\
	export LD_LIBRARY_PATH="${test/RPATH}";\
	for i in ${test/TESTS}; do	\
	    TEST="test/$$(basename $$i)";	\
	    echo "Running $$i";		\
	    ./$$i < $$TEST.cc > $$i.out 2>&1;	\
	    diff $$TEST.std $$i.out && rm -f $$i.out; \
	done

${test/TESTS}: $Otest/%: $Otest/%.o $Otest/stdtest.o ${ALLTGTS}
	@echo "Linking $@ ..."
	@${LD} ${LDFLAGS} -o $@ $< $Otest/stdtest.o ${test/LIBS}

$Otest/.d:	$O.d
	@[ -d $Otest ] || mkdir $Otest
	@touch $Otest/.d

################ Maintenance ###########################################

clean:	test/clean
test/clean:
	@if [ -d $Otest ]; then\
	    rm -f ${test/TESTS} ${test/OBJS} ${test/DEPS} ${test/OUTS} $Otest/stdtest.o $Otest/.d;\
	    rmdir $Otest;\
	fi
check:		test/run
test/check:	check

${test/OBJS} $Otest/stdtest.o:	${MKDEPS} $Otest/.d

-include ${test/DEPS}
