package org.twdata.maven.cli.commands;

import java.util.Iterator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

class CommandTokenizer implements Iterable<String> {
    private final String text;

    public CommandTokenizer(String command) {
        this.text = command;
    }

    public Iterator<String> iterator() {
        return new Iterator<String>() {
            final Matcher m = Pattern.compile("\\s+").matcher(text);
            int next = 0;

            public boolean hasNext() {
                return next < text.length();
            }

            public String next() {
                String s = nextInternal();
                while (s != null && count(s, '"') % 2 == 1 && hasNext()) {
                    s += m.group() + nextInternal();
                }
                return s;
            }

            private String nextInternal() {
                if (!hasNext()) {
                    return null;
                }
                boolean found = m.find(next);
                if (found) {
                    String s = text.substring(next, m.start());
                    next = m.end();
                    return s;
                } else {
                    String s = text.substring(next);
                    next = text.length();
                    return s;
                }
            }

            private int count(final String s, final char ch) {
                int total = 0;
                int pos = s.indexOf(ch);
                while (pos >= 0) {
                    total++;
                    pos = s.indexOf(ch, pos + 1);
                }
                return total;
            }

            public void remove() {
                throw new UnsupportedOperationException();
            }
        };
    }
}
