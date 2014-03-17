package org.twdata.maven.cli;

import org.apache.maven.project.MavenProject;

public class PhaseCallTestDataBuilder {
    private PhaseCall phaseCall = new PhaseCall(false);

    public PhaseCallTestDataBuilder hasPhases(String... phases) {
        for (String phase : phases) {
            phaseCall.addPhase(phase);
        }

        return this;
    }

    public PhaseCall build() {
        return phaseCall;
    }

    public PhaseCallTestDataBuilder runsOffline() {
        phaseCall.goOffline();
        return this;
    }

    public PhaseCallTestDataBuilder hasProjects(MavenProject... projects) {
        for (MavenProject project : projects) {
            phaseCall.addProject(project);
        }

        return this;
    }

    public PhaseCallTestDataBuilder notRecursing() {
        phaseCall.doNotRecurse();
        return this;
    }

    public PhaseCallTestDataBuilder skippingTests() {
        phaseCall.addProperty("maven.test.skip", "true");
        return this;
    }

    public PhaseCallTestDataBuilder hasProfiles(String... profiles) {
        for (String profile : profiles) {
            phaseCall.addProfile(profile);
        }

        return this;
    }

    public PhaseCallTestDataBuilder hasProperties(String... properties) {
        for (String property : properties) {
            String[] propValue = property.split("=");
            phaseCall.addProperty(propValue[0], propValue[1]);
        }

        return this;
    }

    public static PhaseCallTestDataBuilder aPhaseCall() {
        return new PhaseCallTestDataBuilder();
    }
}
