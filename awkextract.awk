# awkextract discards unwanted regenerated script lines
# lines associated with event execution
/\\T/ {
# print event execution
      print $0;
}
# print response time and matched string
/Matched/ {
      print $0;
      next
      }
# lines associated with event declaration
/\\S/ {
      print $0
      }

