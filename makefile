CXX=gcc

all: office_hours

office_hours: officehours.c
	${CXX} -o office_hours officehours.c -pthread

clean:
	rm office_hours