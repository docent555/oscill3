function [tspan, yout] = rkm4(dydt, tspan, y0, options)

if nargin > 3
    v = odeget(options,'OutputFcn');
    eps = odeget(options,'RelTol');
else
    eps = 1e-12;
end

if eps > 0.2 || eps < 1e-12
    error('rkm45: Заданная ошибка вне допустимых границ');
end

status = 0;

if nargin > 3 && ~isempty(v)
    status = v([tspan(1) tspan(end)],y0,'init');
end

if status == 1
    return
end

M = length(tspan);
N = length(y0);
yout = zeros(M,N);
ytmp = zeros(M*5,N);
ttmp = zeros(M*5,1);
c = zeros(N,1);
xx = zeros(N,1);

y = y0;
ytmp(1,:) = y;
hmax = (tspan(2)-tspan(1));
h = hmax/2;
tend = tspan(end);
t = tspan(1);
ttmp(1) = t;
i = 1;

while (t < tend)
    if (t + h) > tend
        h = tend - t;
    end 
    flag = 1;
    while (flag)
        k1 = h * dydt(t, y);
        k2 = h * dydt(t + h / 3, y + k1 / 3);
        k3 = h * dydt(t + h / 3, y + (k1 + k2) / 6);
        
        z = 6 * (k3 - k2) ./ (k2 - k1);
        idx = (z < -3);
        % Для умеренно жёстких уравнений            
        c(idx) = 1./(8 - z(idx)) - 2.5./z(idx).^2 - 4./z(idx).^3 - 1./z(idx).^4;
        % модифицированное значение функции
        xx(idx) = y(idx) + k1(idx) / 2 + c(idx) .* (k2(idx) - k1(idx));
        % Стандартаная оценка функции
        xx(~idx) = y(~idx) + (k1(~idx) + 3 * k3(~idx)) / 8;
        
        k4 = h * dydt(t + h / 2, xx);
        k5 = h * dydt(t + h, y + (k1 - 3 * k3 + 4 * k4) / 2);        
        y = y + (k1 + 4 * k4 + k5) / 6;
        r0 = (2 * k1 - 9 * k3 + 8 * k4 - k5) / 30;
        r = 0; % Абсолютное значение погрешности
        if y ~= 0
            r0 = r0 ./ y;
        end
        r0 = max(abs(r0));
        if abs(r0) > r
            r = abs(r0);
        end
        if r > eps
            % Абсолютная погрешность в пять раз больше относительной
            h = h * 0.5;    
%             disp('small')
        elseif r <= eps / 32
            % Точность интегрирования чрезмерно высока
            h = h * 2;
%             disp('big')
        else
            %Точность интегрирования в заданных пределах
            flag = 0;
        end
    end
    
    t = t + h;
    i = i + 1;
    ytmp(i,:) = y;
    ttmp(i) = t;      
    
    if nargin > 3 && ~isempty(v)
        status = v(t + h,y,[]);
    end
    if status == 1
        ytmp = ytmp(1:i, :);
        tspan = tspan(1:i);
        S = griddedInterpolant(tmp, ytmp(:,1), 'spline');
        yout = S(tspan);
        return
    end       
end

ytmp = ytmp(1:i,:);
ttmp = ttmp(1:i,1);

if nargin > 3 && ~isempty(v)
    status = v([], [] ,'done');
end

for j=1:N
    S = griddedInterpolant(ttmp, ytmp(:,j), 'spline');
    yout(:,j) = S(tspan);
end
return
end
