# gnuger

small GNU/GPL infection detector and replacement list

`gnuger` checks `PATH` and a few standard system paths. It does not crawl the
whole filesystem and does not need root.

## build

```sh
make
make install PREFIX=$HOME/.local
```

## usage

```text
gnuger              infection summary
gnuger -l/list      detected GNU/GPL and non-GNU components
gnuger -e/explain   replacement table with weights and categories
gnuger -r/repl      short replacement list
gnuger -h/help      help
gnuger -v/version   version
```

The score is a weighted share of detected catalog entries:

```text
GNU/GPL points * 100 / (GNU/GPL points + non-GNU points)
```

## license

0BSD, see LICENSE.
