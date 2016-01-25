OBJS = MainStreamed.o TracingStreamed.o CQueueLocation.o test.o
MainStreamed: ${OBJS}
		g++ -o MainStreamed ${OBJS}
clean:
	-rm -rf MainStreamed *.o
