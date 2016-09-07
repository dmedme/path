# awksort sorts all events with the same id together
BEGIN {
x=0;
y=0
}
# Store event declarations (S) in an array
/\\\\S/ {
declare[x]=$0;
x=x+1;
next
}
# Store event execution (T) and match strings in arrays
/\\\\T/ {
event[y]=$0;
next
}
{
result[y]=$0;
y=y+1;
}
END {
# pointer for original array declare
x1=1;
# pointer for new array declare2
x2=1;
# Move first element in as cannot be a duplicate  should test for zero events
declare2[0]=declare[0];
# loop through original array declare for x elements
while(x1<x) {
    dupind=0;
# loop through new array to see if current element of original 
# is a duplicate
    for(x3=0;x3<x2;x3++) {
        if(declare2[x3]==declare[x1])
             dupind=1
        }
# if duplicate then look at next element
    if(dupind==1) {
        x1++ } 
    else  {
# or copy across and reset pointers to both arrays
        declare2[x2]=declare[x1];
        x1++;
        x2++  }
}
# Output event statements grouped together
x=0;
while(x<=x2) {
# event id in character positions 11 & 12
             id=substr(declare2[x],11,2);
# print event declaration
             print id ":" declare2[x];
             y2=0;
             while(y2<=y) {
                          ide=substr(event[y2],11,2);
                          if (ide==id) {
# print result of event, response time and matched string
                                       print result[y2]
                                       }
                          y2++
                          }
             x++
             }
}
