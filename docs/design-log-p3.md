# Project 3 Design Log
**Author:** <Charlie Levine> · **P2 code used:** <instructor reference>
**Tools used:** <AI assistants / Clion>
I find that the "autofill" code for Clion is much better than vscode. It always matches my variable names correctly, 
creates my for loops with the bounds I need based older functions, and it has a "sweep" button where I can clean the spacing.
I find that when I program in Clion (then paste over), I am faster and I don't need to go back and figure out how many versions
of the same variable name that I have.


## 1. Hash table
     Separate chaining (std::vector) is a more stable way to handle absolute pointer. Only the tool's pointer moves when the "chain" changes shape, and the element memory is dynamic (though I'm sure that would not be an issue for this program size). I don't have to fish around for element pointers, and I know that each bucket index has its vector.

     My hash function is FNV-1a (defined in the specs) with a 0.75 load factor. FNV utilizes every byte in order. 0.75 triggers a resize when adding pushes past the limit, doubling the buckets. 
     TERMINAL OUT:
     buckets: 16384, load factor: 0.610, longest chain: 6 
     at 16384 buckets, longest chain: 6, weak longest chain: 9000
     This is a good indication that the current state is optimized: at 16,384 buckets and a 0.610 load factor, a maximum chain length of 6 desribes balanced distribution. The `weak longest chain: 9000` is maximum chain from before the resize operations redistributes. This means that over 9,000 elements were in a tiny bucket allocation before the table doubled to 16,384 buckets and redistributed the keys evenly.
     
     Add is amortized to keep the huge amount of insertions constant. A rehash (O(n)) is expensive when the hash table doubles.
## 2. Pointer stability
Separate chaining allows a separeate vecor of individual bucket buckets. The tool object never moves off of the heap, even when the pointer moves. This way, a Tool* always survives a resize. 

## 3. Calculator trace

Tracing `2 * (3 + 4) - 5`: 

1. sees the 2, pushes the number, operation absent
2. sees the *, operation empty, pushes
3. sees the (, pushes as op next to *
4. sees the 3, pushes with the 2
5. sees the +, but the ) is missing. pushes
6. sees the 4, pushes
7. sees the ), ending parentheses pops the 4 to the right, and the 3 to the left (evaluates the 7), pushes 7, then sees the hanging (, and pops it
8. sees the -, precedence ignores it because * has higher priority. pushes and applies the 2*7 to push
9. sees the 5, pushes number
10. last operation is -. applies 14-5 = 9


## 4. Errors as values

Using an error as a "value" allows the program to compute the error case at compiler time instead of going back and sifting through the call stack. It sees it as just another result. You can easily follow an input through the path of execution.

Exceptions should be saved for unpredicable behavior that may end up manipulating memory due to shortages or violating some other behavior. 
## 5. What I would design differently
My main issues came from my shaky understanding of the language model developed in P2. It took me a very long time to pass the same tests that previously worked, likely due version push errors. Eventually, I settled on using the project 2 example code, which was a much cleaner version to build on. Also, I switched everything from linked lists to vectors, because they are way less complicated. 

