#!/bin/bash
RANDOM=$$
for i in {0..4}
{
	./cli 127.0.0.1 8888 $(((RANDOM%10)+1)) $((i))&
}
exit 0
