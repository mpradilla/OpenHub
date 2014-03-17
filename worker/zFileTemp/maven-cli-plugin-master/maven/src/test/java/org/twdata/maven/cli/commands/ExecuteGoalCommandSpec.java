package org.twdata.maven.cli.commands;

import java.util.HashMap;
import jdave.Specification;
import jdave.junit4.JDaveRunner;
import org.junit.runner.RunWith;

@RunWith(JDaveRunner.class)
public class ExecuteGoalCommandSpec extends Specification<Void> {
    public class WhenMatchingRequests {
        private ExecuteGoalCommand command;

        public void create() {
            HashMap<String, String> userDefinedGoal = new HashMap<String, String>();
            userDefinedGoal.put("all", "clean compile test");

            command = new ExecuteGoalCommand(null, null, null, userDefinedGoal);
        }

        public void shouldMatchWhenSpecifiedRequestIsOneOfThePredefinedGoals() {
            specify(command.matchesRequest("clean"), should.equal(true));
        }

        public void shouldMatchWhenSpecifiedRequestIsOneOfTheUserDefinedGoals() {
            specify(command.matchesRequest("all"), should.equal(true));
        }

        public void shouldMatchWhenSpecifiedRequestRunningASpecifiedPluginGoal() {
            specify(command.matchesRequest("group:artifact:goal"), should.equal(true));
        }

        public void shouldNotMatchWhenRequestForSpecifiedPluginOmitsGoal() {
            specify(command.matchesRequest("group:artifact"), should.equal(false));
        }
    }
}
