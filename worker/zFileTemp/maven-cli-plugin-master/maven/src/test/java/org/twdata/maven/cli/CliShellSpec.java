package org.twdata.maven.cli;

import java.util.ArrayList;
import jdave.Specification;
import jdave.junit4.JDaveRunner;
import org.jmock.Expectations;
import org.junit.runner.RunWith;
import org.twdata.maven.cli.commands.Command;
import org.twdata.maven.cli.console.CliConsole;

@RunWith(JDaveRunner.class)
public class CliShellSpec extends Specification<Void> {
    public class WhenRunningACommand {
        private ArrayList<Command> commands = new ArrayList<Command>();
        private Command mockCommand = mock(Command.class);
        private CliConsole mockConsole = mock(CliConsole.class);

        public void create() {
            commands.add(mockCommand);
        }

        public void shouldNotExitLoopIfTheCommandRaisesException() {
            CliShell shell = new CliShell(commands, mockConsole);

            checking(new Expectations() {{
                allowing(same(mockConsole)).method("write.*").with(anything());

                exactly(2).of(mockConsole).readLine();
                will(onConsecutiveCalls(returnValue("abcd"), returnValue("exit")));

                allowing(same(mockCommand)).method("matchesRequest").with(anything());
                will(returnValue(true));

                allowing(mockCommand).run("abcd", mockConsole);
                will(throwException(new RuntimeException("test")));
                allowing(mockCommand).run("exit", mockConsole);
                will(returnValue(false));
            }});

            shell.run();
        }
    }
}
