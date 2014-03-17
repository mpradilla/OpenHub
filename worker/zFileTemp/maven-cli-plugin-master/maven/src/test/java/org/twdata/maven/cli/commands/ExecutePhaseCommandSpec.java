package org.twdata.maven.cli.commands;

import org.twdata.maven.cli.*;
import java.util.HashSet;
import java.util.Set;
import jdave.Specification;
import jdave.junit4.JDaveRunner;
import org.junit.runner.RunWith;

@RunWith(JDaveRunner.class)
public class ExecutePhaseCommandSpec extends Specification<Void> {
    public class WhenRunningRequests {
        private Set<String> modules = new HashSet<String>();
        private Set<String> userAliases = new HashSet<String>();
        private PhaseCallBuilder mockBuilder = mock(PhaseCallBuilder.class);
        private PhaseCallRunner mockRunner = mock(PhaseCallRunner.class);
        private ExecutePhaseCommand command =
                new ExecutePhaseCommand(userAliases, modules, mockBuilder, mockRunner, false);

        public void shouldNotMatchRequestIfAnyTheTokensIsInvalid() {
            specify(command.matchesRequest("-o build"), should.equal(false));
        }

        public void shouldNotRejectKeyValuesOfProperties() {
            specify(command.matchesRequest("-Dabcd=def"), should.equal(true));
        }

        public void shouldNotRejectQuotedPropertyValuesWithWhitespace() {
            specify(command.matchesRequest("-Dabcd=\"def ghi\""), should.equal(true));
        }

        public void shouldNotRejectProfileInput() {
            specify(command.matchesRequest("-Pprofile"), should.equal(true));
        }

        public void shouldNotRejectUserAliasesInput() {
            userAliases.add("alias");
            specify(command.matchesRequest("alias"), should.equal(true));
        }

        public void shouldNotRejectIfMatchesModuleWildCardInput() {
            modules.add("module");
            specify(command.matchesRequest("mod*"), should.equal(true));
        }

        public void shouldNotRaiseErrorWhenInputIsQuestionMarkWhenModuleExists() {
            modules.add("module");
            command.matchesRequest("?");
        }

        public void shouldNotRejectPluginGoalsWithExplicitGroupId() {
            specify(command.matchesRequest("groupId:pluginId:goal"), should.equal(true));
        }

        public void shouldNotRejectPluginGoalsWithNoExplicitGroupId() {
            specify(command.matchesRequest("pluginId:goal"), should.equal(true));
        }

        public void shouldRejectPluginGoalsWithNoExplicitGroupIdAndPluginId() {
            specify(command.matchesRequest(":goal"), should.equal(false));
        }

        public void shouldRejectPluginGoalsWithNoExplicitGroupIdAndGoal() {
            specify(command.matchesRequest("pluginId:"), should.equal(false));
        }

        public void shouldRejectPluginGoalsWithNoExplicitGroupIdAndPluginAndGoal() {
            specify(command.matchesRequest("::"), should.equal(false));
        }
    }
}
