package org.twdata.maven.cli;

import java.util.HashSet;
import jdave.Specification;
import jdave.junit4.JDaveRunner;
import org.junit.runner.RunWith;

@RunWith(JDaveRunner.class)
public class CommandTokenCollectorSpec extends Specification<Void> {
    public class WhenCollectingTokens {
        private CommandTokenCollector collector = new CommandTokenCollector();

        public void shouldCollectAllNonFlagLikeTokens() {
            HashSet<String> tokens = new HashSet<String>();
            tokens.add("token");
            tokens.add("-flag");

            collector.addCommandTokens(tokens);
            specify(collector.getCollectedTokens(), should.containAll("token"));
        }
    }
}
