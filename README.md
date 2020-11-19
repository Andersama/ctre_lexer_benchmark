# ctre_lexer_benchmark

A quick test of different lexing approaches. Unfortuantely while ctre is fantastic for generating native code that you'd have otherwise written by hand, it can be outdone with a dfa approach.

Some not so great testing results...where my naive attempts blow ctre out of the water.

Note: not exactly a complete test suite, here I'm comparing approaches to lex whitespace and non-whitespace characters. The dfa is extremely small.
```
| relative |             ns/byte |              byte/s |    err% |     total | benchmark
|---------:|--------------------:|--------------------:|--------:|----------:|:----------
|   100.0% |                2.12 |      472,049,817.08 |    0.7% |      0.32 | `hand_coded table`
|   214.2% |                0.99 |    1,011,189,413.06 |    1.6% |      0.35 | `hand_coded table sse`
|    97.1% |                2.18 |      458,505,050.23 |    2.7% |      0.32 | `hand_coded switch inside if`
|    94.9% |                2.23 |      447,839,585.40 |    3.9% |      0.33 | `hand_coded switch only in loop`
|    60.6% |                3.50 |      285,960,135.45 |    1.4% |      0.35 | `hand_coded break to switch`
|    11.8% |               17.97 |       55,642,484.11 |    0.7% |      0.66 | `ctre::starts_with`
|     8.1% |               26.04 |       38,396,531.90 |    0.1% |      0.92 | `ctre::starts_with overlap_strs`
```

I absolutely hate and detest having to write tables by hand. So I'm very motivated to improve on ctre. Since regexs can be represented as dfa's I don't see any particular reason why there shouldn't be an equivalent ctdfa library. In fact in having played around with the internals of the ctre library a few times it seems extremely tempting to rip the existing compile time parser for the purpose of building dfas.

In testing I've already found a significant improvement in searching for a string based pattern by transforming some strings info a prefix dfa matcher. A lot of the current results are mostly an unfortunate consequence of branch misprediction, otherwise the native code representing the regex should be a clear win. Another option would be to perhaps make a "helper" ctre function to generate something more akin to an optimal lexer, much like "starts_with". Don't want to bother Hana though.

I've picked up a few new tricks from https://branchfree.org/, quite frankly my original lexers were quite a mess of a combination of the two approaches, and I'd have been happy to have results akin to `ctre:::starts_with` here. However that lexer does significantly more work for an even more complicated regular expression, it keeps track of rows and columns, de-escapes strings and adds them to a hash table database and peforms both string to integer and string to floating point operations. It's difficult to gauge how good my lexer is without peformance benchmarks of other lexers built to tackle the same problem space. The ones I've found like for the RE2 library and even for tinyC lead me to think I'm close, but at the same time given the results touted by simdjson, simdcsv and hyperscan lead me to think otherwise.

Another consideration is I've not dealt with or touched anything to do w/ utf8. There's an obvious cost to the dfa table size if the dfa is meant to handle utf8 sequences. It's not entirely obvious to me whether processing input in utf8 form is worth it, the variable size would seem to mean that there'd need to be two loops, which would add another branch predicition issue into the mix.

Ideally I'd have a library that:
- Can represent a language as a series of regexs at compile time
- Can generate some near optimal code for a lexer

I can live with post processing tokens, I'd much rather have a blazing fast lexer / parser in my tool belt.

Note to self: Just spotted dfa branch in the ctre library* will be taking a look at that.