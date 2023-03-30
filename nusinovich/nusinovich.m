clearvars
D = 0.3;
eta = 0.4;
% L = 2 + 0.001731601731602  * 316;
% mu = 10 + 0.012360939431397  * 394;
mu = 23.1;
% Inus = 10^L;
Inus = 794;
Inov = 2*Inus/mu^4;

Ne = 32;
zex = 40;

Fnus = sqrt(eta*power(mu, -4)*Inus);
Fnov = sqrt(Inov*eta/2)

Nz = 1001;

ZetaAxis_nus = -sqrt(3)/2*mu:sqrt(3)*mu/(Nz-1):sqrt(3)/2*mu;
u_nus = exp(-(2*ZetaAxis_nus/mu).^2);

ZetaAxis_nov = 0:zex/(Nz-1):zex;
u_nov = exp(-3*(ZetaAxis_nov - zex/2).^2/(zex/2)^2);

% plot(u_nus); hold on; plot(u_nov)

Th0 = (2*pi*(0:Ne-1)/Ne)';
P0 = exp(1i*Th0);

SUnus = griddedInterpolant(ZetaAxis_nus, u_nus);
SUnov = griddedInterpolant(ZetaAxis_nov, u_nov);

% plot(SUnus(ZetaAxis_nus)); hold on; plot(SUnov(ZetaAxis_nov))

osol = @ode4;

[~, Pnus] = osol(@(z, x) MomentumODE(z, x, Fnus, SUnus, D), ZetaAxis_nus, P0);

eff_nus1 = Efficiency(Pnus(end,:), Ne);

[~, Pnov] = osol(@(z, y) MomentumODEnov(z, y, Fnov, SUnov, D, mu), ZetaAxis_nov, P0);

eff_nov1 = Efficiency(Pnov(end,:), Ne);

% hf=figure;
% for i=1:Ne
%     plot(real(Pnus(:,i))); hold on; plot(real(Pnov(:,i)))
%     pause
%     clf
% end
% hold off
% close(hf)

P0v = [real(P0); imag(P0)];

Idx.Re = 1:Ne;
Idx.Im = Ne + Idx.Re;

[~, Pv_nus] = osol(@(z, Pv) MomentumODEv(z, Pv, Fnus, SUnus, D, Idx), ZetaAxis_nus, P0v);

Pnus2(:,:) = Pv_nus(:,Idx.Re) + 1i*Pv_nus(:,Idx.Im);

eff_nus2 = Efficiency(Pnus2(end,:), Ne);

[~, Pv_nov] = osol(@(z, Pv) MomentumODEvnov(z, Pv, Fnov, SUnov, D, Idx, mu), ZetaAxis_nov, P0v);

Pnov2(:,:) = Pv_nov(:,Idx.Re) + 1i*Pv_nov(:,Idx.Im);

eff_nov2 = Efficiency(Pnov2(end,:), Ne);

fprintf('Nusinovich Efficency compl = %g\n', eff_nus1);
fprintf('Nusinovich Efficency real = %g\n', eff_nus2);
fprintf('Novozhilova Efficency compl = %g\n', eff_nov1);
fprintf('Novozhilova Efficency real = %g\n', eff_nov2);