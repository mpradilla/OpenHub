package org.twdata.maven.cli.commands;

import jdave.Specification;
import jdave.junit4.JDaveRunner;
import org.jmock.Expectations;
import org.junit.runner.RunWith;
import org.twdata.maven.cli.console.CliConsole;
import static org.hamcrest.Matchers.equalToIgnoringWhiteSpace;

@RunWith(JDaveRunner.class)
public class CliConsoleCommandDescriptionSpec extends Specification<Void> {
    private CliConsole mockConsole = mock(CliConsole.class);
    private CliConsoleCommandDescription description = new CliConsoleCommandDescription(mockConsole);

    public class WhenNothingIsDescribed {
        public void shouldPrintAnEmptyString() {
            checking(new Expectations() {{
                exactly(1).of(same(mockConsole)).method("writeInfo")
                        .with(equalToIgnoringWhiteSpace(""));
            }});

            description.outputDescription();
        }
    }

    public class WhenDescribingACommand {
        public void shouldDescribeTheCommandNameWithAnAppendedColon() {
            checking(new Expectations() {{
                exactly(1).of(same(mockConsole)).method("writeInfo")
                        .with(equalToIgnoringWhiteSpace("Command name:"));
            }});

            description.describeCommandName("Command name");
            description.outputDescription();
        }

        public void shouldDescribeTheCommandTokenEvenWhenItsAccompanyingDescriptionIsNull() {
            checking(new Expectations() {{
                exactly(1).of(same(mockConsole)).method("writeInfo")
                        .with(equalToIgnoringWhiteSpace("token"));
            }});

            description.describeCommandToken("token", null);
            description.outputDescription();
        }

        public void shouldDescribeTheCommandNameAndCommandTokenAndAccompanyingDescription() {
            checking(new Expectations() {{
                exactly(1).of(same(mockConsole)).method("writeInfo")
                        .with(equalToIgnoringWhiteSpace("Command name: token description"));
            }});

            description.describeCommandName("Command name");
            description.describeCommandToken("token", "description");

            description.outputDescription();
        }
    }

    public class WhenTwoCommandsAreDescribed {
        public void shouldDescribeEverythingFromTheTwoGivenCommands() {
            checking(new Expectations() {{
                exactly(1).of(same(mockConsole)).method("writeInfo")
                        .with(equalToIgnoringWhiteSpace("Command123: token123 description123 " +
                        "Command456: token456 description456"));
            }});

            description.describeCommandName("Command123");
            description.describeCommandToken("token123", "description123");
            description.describeCommandName("Command456");
            description.describeCommandToken("token456", "description456");

            description.outputDescription();
        }
    }
}
