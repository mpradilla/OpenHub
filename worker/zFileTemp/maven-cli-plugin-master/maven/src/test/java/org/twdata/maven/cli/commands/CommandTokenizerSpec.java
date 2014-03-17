package org.twdata.maven.cli.commands;

import org.junit.runner.RunWith;

import jdave.Specification;
import jdave.junit4.JDaveRunner;

@RunWith(JDaveRunner.class)
public class CommandTokenizerSpec extends Specification<CommandTokenizer> {
    public class WhenInputIsEmpty {
        public void willReturnEmptyIterator() {
            specify(new CommandTokenizer("").iterator(), containsExactly());
        }
    }

    public class WhenInputIsSingleWord {
        public void willReturnTheWord() {
            specify(new CommandTokenizer("package"), containsExactly("package"));
        }
    }

    public class WhenInputContainsPhasesSwitchesGoalsAndProperties {
        public void willSplitOnSpaces() {
            specify(new CommandTokenizer("clean release:prepare -o -Dmaven.test.skip=true"),
                    containsExactly("clean", "release:prepare", "-o", "-Dmaven.test.skip=true"));
        }

        public void willSplitOnMultipleSpaces() {
            specify(new CommandTokenizer("clean  release:prepare   -o    -Dmaven.test.skip=true"),
                    containsExactly("clean", "release:prepare", "-o", "-Dmaven.test.skip=true"));
        }

        public void willSplitOnTabs() {
            specify(new CommandTokenizer("clean\trelease:prepare\t-o\t-Dmaven.test.skip=true"),
                    containsExactly("clean", "release:prepare", "-o", "-Dmaven.test.skip=true"));
        }
    }

    public class WhenInputContainsQuotedStringsWithSpaces {
        public void willNotSplitOnQuotedSpaces() {
            specify(new CommandTokenizer("help:active-profiles -Dxyz=\"my test\""),
                    containsExactly("help:active-profiles", "-Dxyz=\"my test\""));
        }

        public void willPreserveMultipleQuotedSpaces() {
            specify(new CommandTokenizer("help:active-profiles -Dxyz=\"my   test\""),
                    containsExactly("help:active-profiles", "-Dxyz=\"my   test\""));
        }

        public void willPreserveMultipleQuotedTabs() {
            specify(new CommandTokenizer("help:active-profiles -Dxyz=\"my\t\ttest\""),
                    containsExactly("help:active-profiles", "-Dxyz=\"my\t\ttest\""));
        }

        public void willPreserveMultipleQuotedNewlines() {
            specify(new CommandTokenizer("help:active-profiles -Dxyz=\"my\ntest\n\""),
                    containsExactly("help:active-profiles", "-Dxyz=\"my\ntest\n\""));
        }
    }
}
