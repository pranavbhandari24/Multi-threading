# Multi-threading
This program is a simulation of a professors office hours to show hands-on experience in parallel programming.

# Description
The professor is teaching 2 classes this semester, class A and
class B, and is holding shared office hours for both classes in his office. The professor can
have a large number of students showing up for his office hours, so he decides to impose
several restrictions. 
1) The professors's office has only 3 seats, so no more than 3 students are
   allowed to simultaneously enter the professor’s office. When the office is full and new
   students arrive they have to wait outside the office.
2) The professor gets confused when helping students from class A and
   class B at the same time. He decides that while students from class A are in his office, no
   students from class B are allowed to enter, and the other way around. 
3) The professor gets tired after answering too many questions. He decides
   that after helping 10 students he needs to take a break before he can help more students.
   So after the 10th student (counting since the last break) enters the professors office no
   more students are admitted into the office, until after the professors's next break. Students
   that arrive while the professor is taking his break have to wait outside the office.
4) In order to be fair to both classes after 5 consecutive students from a
   single class the professor will answer questions from a student from the other class. 
5) Your program should ensure progress, i.e. if there is no student in the
   professor’s office and the professor is not currently taking a break an arriving student
   should not be forced to wait. Similarly, if an arriving student is compatible with the
   students currently in the office he should not be forced to wait, unless the professor is due
   for a break. 
   
# Format of Input File
The program requires a input file to run. This input file should contain the Class of the student (A or B), the arrival time and the question time in the following format :

                  Class   Arrival_Time  Question_Time

The class value is represented by 0 and 1, where 0 is class A and 1 is class B. There should be details about only 1 student in 1 line. A sample file is uploaded.
  
# How to run:
To run the program type the following commands:
  make
  ./office_hours <name of input file>

