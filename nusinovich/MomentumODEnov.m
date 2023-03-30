function dPdz = MomentumODEnov(z, P, F, SU, D, mu)

kappa = sqrt(3)*mu/40;
% kappa = 1;

dPdz(:,1) = 1i*kappa*(F * SU(z) - (D + abs(P(:,1)).^2 - 1).*P(:,1));

end

