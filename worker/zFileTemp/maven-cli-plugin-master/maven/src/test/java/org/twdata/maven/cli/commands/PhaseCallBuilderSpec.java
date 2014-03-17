package org.twdata.maven.cli.commands;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import jdave.Specification;
import jdave.junit4.JDaveRunner;
import org.apache.maven.project.MavenProject;
import org.junit.runner.RunWith;
import org.twdata.maven.cli.PhaseCall;
import org.twdata.maven.cli.PhaseCallTestDataBuilder;
import static org.unitils.reflectionassert.ReflectionAssert.assertReflectionEquals;
import static org.twdata.maven.cli.PhaseCallTestDataBuilder.aPhaseCall;

@RunWith(JDaveRunner.class)
public class PhaseCallBuilderSpec extends Specification<PhaseCallBuilder> {
    private MavenProject defaultProject = new MavenProject();
    private Map<String, MavenProject> modules = new HashMap<String, MavenProject>();
    private Map<String, String> userAliases = new HashMap<String, String>();
    private PhaseCallBuilder builder =
            new PhaseCallBuilder(defaultProject, modules, userAliases, false);

    public class WhenInputtingWithExtraSpaces {
        public void willRemoveSpacesInParsing() {
            assertPhaseCalls(builder.parseCommand("clean  -o  test"),
                    aPhaseCall().hasPhases("clean", "test").runsOffline().hasProjects(defaultProject));
        }
    }

    public class WhenInputOnlyContainsPhases {
        public void shouldBuildOnlyOneCommandContainingAllThePhases() {
            assertPhaseCalls(builder.parseCommand("clean test"),
                    aPhaseCall().hasPhases("clean", "test").hasProjects(defaultProject));
        }
    }

    public class WhenInputContainsPhasesAndOfflineSwitch {
        public void shouldBuildOnlyOneCommandWhenSwitchIsSpecifiedAfterPhases() {
            assertPhaseCalls(builder.parseCommand("clean test -o"),
                    aPhaseCall().hasPhases("clean", "test").runsOffline()
                            .hasProjects(defaultProject));
        }

        public void shouldBuildTwoCommandWhenSwitchIsSpecifiedBeforePhases() {
            assertPhaseCalls(builder.parseCommand("-o clean test"),
                    aPhaseCall().runsOffline(),
                    aPhaseCall().hasPhases("clean", "test").hasProjects(defaultProject));
        }

        public void shouldBuildOneCommandEvenWhenSwitchIsSpecifiedBetweenPhases() {
            assertPhaseCalls(builder.parseCommand("clean -o test"),
                    aPhaseCall().hasPhases("clean", "test").runsOffline()
                            .hasProjects(defaultProject));
        }
    }

    public class WhenQuotesArePresent {
        public void willBuildOneCommandWhenPropertyHasNoWhitespace() {
            assertPhaseCalls(builder.parseCommand("-Dabc=\"def\""), aPhaseCall().hasProperties("abc=\"def\""));
        }
        public void willBuildOneCommandWhenPropertyHasWhitespacePreserved() {
            assertPhaseCalls(builder.parseCommand("-Dabc=\"def \n ghi\""), aPhaseCall().hasProperties("abc=\"def \n ghi\""));
        }
        public void willBuildOneCommandWhenTwoPropertiesHaveWhitespacePreserved() {
            assertPhaseCalls(builder.parseCommand("-Dabc=\"def \n ghi\" -Djkl=\"mno pqr\""),
                    aPhaseCall().hasProperties("abc=\"def \n ghi\""),
                    aPhaseCall().hasProperties("jkl=\"mno pqr\""));
        }
    }

    public class WhenOnlySwitchesAreSpecified {
        public void willBuildOneCommandWhenOnlyOfflineSwitchIsSpecified() {
            assertPhaseCalls(builder.parseCommand("-o"), aPhaseCall().runsOffline());
        }

        public void willBuildOneCommandWhenOnlyDoNotRecurseSwitchIsSpecified() {
            assertPhaseCalls(builder.parseCommand("-N"), aPhaseCall().notRecursing());
        }

        public void willBuildOneCommandWhenOnlySkipTestsSwitchIsSpecified() {
            assertPhaseCalls(builder.parseCommand("-S"), aPhaseCall().skippingTests());
        }

        public void willBuildOneCommandWhenOnlyProfileSwitchIsSpecified() {
            assertPhaseCalls(builder.parseCommand("-Pprofile"), aPhaseCall().hasProfiles("profile"));
        }

        public void willBuildOneCommandWhenOnlyPropertySwitchIsSpecified() {
            assertPhaseCalls(builder.parseCommand("-Dabcd=def"),
                    aPhaseCall().hasProperties("abcd=def"));
        }

        public void willBuildTwoCommandsWhenTwoPropertiesAreSpecified() {
            assertPhaseCalls(builder.parseCommand("-Dabcd=def -Ddefg=ghi"),
                    aPhaseCall().hasProperties("abcd=def"),
                    aPhaseCall().hasProperties("defg=ghi"));
        }

        public void willBuildTwoCommandsWhenTwoSwitchesAreSpecified() {
            assertPhaseCalls(builder.parseCommand("-Dabcd=def -o"),
                    aPhaseCall().hasProperties("abcd=def"),
                    aPhaseCall().runsOffline());
        }

        public void willBuildOneCommandWhenPropertySwitchWithAsteriskIsSpecified() {
            assertPhaseCalls(builder.parseCommand("-Dabcd=def*"),
                    aPhaseCall().hasProperties("abcd=def*"));
        }

        public void willBuildNothingWhenNoKeyValueSpecifiedAfterPropertySwitch() {
            specify(builder.parseCommand("-D").size(), should.equal(0));
        }

        public void willBuildNothingWhenNoValueIsSpecifiedAfterPropertySwitch() {
            specify(builder.parseCommand("-Dabc").size(), should.equal(0));
        }

        public void willBuildNothingWhenNoValueIsSpecifiedAfterPropertySwitchWithEqualSign() {
            specify(builder.parseCommand("-Dabc=").size(), should.equal(0));
        }

        public void willBuildNothingWhenNoKeyIsSpecifiedAfterPropertySwitch() {
            specify(builder.parseCommand("-D=def").size(), should.equal(0));
        }
    }

    public class WhenSpecifyingModules {
        public void willBuildCommandWithNoValidLifecyleToRunIfInputHasNoPhasesSpecified() {
            MavenProject submodule = new MavenProject();
            modules.put("module", submodule);

            assertPhaseCalls(builder.parseCommand("module"), aPhaseCall().hasProjects(submodule));
        }

        public void willBuildCommandWithModulesMatchingTheWildCardSpecified() {
            MavenProject submodule1 = new MavenProject();
            MavenProject submodule2 = new MavenProject();

            modules.put("submodule1", submodule1);
            modules.put("submodule2", submodule1);
            modules.put("module", new MavenProject());

            assertPhaseCalls(builder.parseCommand("sub*"),
                    aPhaseCall().hasProjects(submodule1, submodule2));
        }

        public void willNotBuildCommandIfWildCardSpecifiedMatchesNothing() {
            modules.put("mod1", new MavenProject());
            modules.put("mod2", new MavenProject());

            specify(builder.parseCommand("sub*").size(), should.equal(0));
        }

        public void willBuildCommandWithPhasesSpecifiedAfterTheModuleName() {
            MavenProject submodule = new MavenProject();
            modules.put("mod1", submodule);

            assertPhaseCalls(builder.parseCommand("clean mod1 test"),
                    aPhaseCall().hasProjects(defaultProject).hasPhases("clean"),
                    aPhaseCall().hasProjects(submodule).hasPhases("test"));
        }
    }

    public class WhenUserAliasesAreSpecified {
        public void willExpandAliasAsDefinedInPom() {
            userAliases.put("explode", "clean compile");

            assertPhaseCalls(builder.parseCommand("explode"),
                    aPhaseCall().hasPhases("clean", "compile").hasProjects(defaultProject));
        }

        public void willJoinTheUsualParsingAfterExpandingTheAlias() {
            userAliases.put("explode", "clean compile");

            assertPhaseCalls(builder.parseCommand("explode -N test"),
                    aPhaseCall().hasPhases("clean", "compile", "test").hasProjects(defaultProject)
                        .notRecursing());
        }

        public void willResolveEmbeddedAliases() {
            userAliases.put("explode", "clean compile");
            userAliases.put("clean-test", "explode test");

            assertPhaseCalls(builder.parseCommand("clean-test"),
                    aPhaseCall().hasPhases("clean", "compile", "test").hasProjects(defaultProject));
        }
    }

    public class WhenProfileSwitchIsSpecified {
        public void willBuildCommandEvenIfNoPhasesIsSpecified() {
            assertPhaseCalls(builder.parseCommand("-Pprofile"), aPhaseCall().hasProfiles("profile"));
        }

        public void shouldNotBuildCommandIfProfileIsNotSpecifiedAfterTheSwitch() {
            specify(builder.parseCommand("-P").size(), should.equal(0));
        }
    }

    private void assertPhaseCalls(List<PhaseCall> actual,
            PhaseCallTestDataBuilder... expected) {
        specify(actual.size(), should.equal(expected.length));

        for (int i = 0; i < actual.size(); i++) {
            assertReflectionEquals(expected[i].build(), actual.get(i));
        }
    }
}
