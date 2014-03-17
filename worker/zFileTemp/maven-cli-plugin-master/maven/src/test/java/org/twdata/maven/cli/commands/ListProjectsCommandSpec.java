package org.twdata.maven.cli.commands;

import java.util.HashSet;
import jdave.Specification;
import jdave.junit4.JDaveRunner;
import org.jmock.Expectations;
import org.junit.runner.RunWith;
import org.twdata.maven.cli.console.CliConsole;

@RunWith(JDaveRunner.class)
public class ListProjectsCommandSpec extends Specification<Void> {
    public class WhenListingTheProjects {
        public void shouldOutputToCliConsole() {
            final HashSet<String> names = new HashSet<String>();
            names.add("name1");
            names.add("name2");

            final CliConsole mockConsole = mock(CliConsole.class);

            checking(new Expectations() {{
                atLeast(names.size()).of(same(mockConsole)).method("writeInfo")
                        .with(any(String.class));
            }});

            new ListProjectsCommand(names).run(null, mockConsole);
        }
    }
}
